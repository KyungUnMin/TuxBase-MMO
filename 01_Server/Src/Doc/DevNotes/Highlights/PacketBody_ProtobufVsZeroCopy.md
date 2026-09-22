# 패킷 바디 설계: 완전 Zero-Copy 대신 Protobuf를 택한 이유

## 1. 배경

원래 계획은 패킷 구조를 밑바닥부터 직접 설계해서, 헤더뿐 아니라 **바디까지 포함해 완전히 Zero-Copy로 처리**하는 것이었다. `RingBuffer`(`01_ServerBase/Include/DataStruct/RingBuffer.h`) 자체는 이 목표에 맞게 설계돼 있다 — `RingBufferReader::As<T>()`로 버퍼 메모리를 그대로 포인터 캐스팅해서 쓰는, 복사가 전혀 없는 접근 방식이다.

하지만 실제로는 패킷 바디 직렬화에 Protobuf(`google::protobuf::Message`)를 쓰기로 했고, 그 순간부터 "바디까지 포함한 완전 Zero-Copy"는 성립하지 않게 됐다.

## 2. 문제

- `PacketBody`(`= google::protobuf::Message`)는 추상 클래스이고, 내부에 `std::string`/repeated 필드 등 가변 길이 데이터를 힙에 따로 들고 있는 비POD 객체다.
- Protobuf의 wire 인코딩(varint, tag-length-value 등)은 C++ 객체의 메모리 레이아웃과 다르다.
- 그래서 수신 측에서 링버퍼 위의 바이트를 `reinterpret_cast`해서 곧바로 쓸 수 없고, 반드시 `Message::ParseFromArray()`로 **역직렬화(파싱)해서 새 객체를 생성**해야 한다 (`PacketSerializer.h`의 `Read()`).
- 송신 측도 마찬가지로 `SerializeToArray()`로 링버퍼 메모리에 직접 인코딩해 넣긴 하지만(`PacketSerializer.h:29`), 이건 "포인터만 넘기는" 것과는 다른, 직렬화 연산이다.

## 3. 선택: 계층별로 Zero-Copy 범위를 나눈다

전부 포기하는 대신, **어디까지 Zero-Copy이고 어디서부터 트레이드오프인지를 계층으로 명확히 구분**했다.

| 계층 | Zero-Copy 여부 | 근거 |
|---|---|---|
| 소켓 recv/send ↔ `RingBuffer` | ✅ 완전 Zero-Copy | `RingBufferWriter`/`RingBufferReader`가 버퍼 메모리를 직접 노출, 중간 복사 버퍼 없음 |
| 패킷 헤더(`PacketHeader`, 4바이트) 파싱 | ✅ 완전 Zero-Copy | `PacketSerializer::PeekHeader()`가 `reader.As<PacketHeader>()`로 포인터 캐스팅만 함 |
| 패킷 바디(Protobuf) 직렬화/역직렬화 | ❌ Zero-Copy 아님 | `SerializeToArray`/`ParseFromArray`가 인코딩/디코딩 연산을 수행 — 바디는 항상 새 객체로 생성됨 |

즉 네트워크 계층(버퍼링, 프레이밍, 헤더 처리)은 여전히 Zero-Copy를 그대로 어필할 수 있는 지점이고, 바디 직렬화만 스키마 기반 방식(Protobuf)으로 트레이드오프를 택한 것이다.

## 4. 왜 완전 Zero-Copy를 포기했는가

| | 자체 설계 POD 구조체 (완전 Zero-Copy) | Protobuf (현재 선택) |
|---|---|---|
| Zero-Copy | 헤더+바디 전부 포인터 캐스팅 가능 | 헤더까지만, 바디는 파싱 필요 |
| 멀티 언어 확장성 | 바이너리 레이아웃(패딩, endianness, 가변 길이 필드)을 언어별로 직접 맞춰야 함 — 사실상 자체 프로토콜을 새로 설계하는 것과 같음 | 스키마(.proto) 기반이라 C++/Node.js 등 다른 언어에서도 공식 바인딩으로 바로 사용 가능 |
| 가변 길이 필드(문자열, 리스트 등) | 직접 규칙을 설계해야 함 | 기본 지원 |
| 유지보수 | 필드 추가/변경 시 각 언어 파싱 코드를 수동으로 맞춰야 함 | `.proto` 재생성만으로 언어 간 동기화 |

로그인/빌링/DB 쿼리 서버를 Node.js로 만들 가능성이 있는 상황(`CLAUDE.md` 향후 계획 참고)에서, 자체 바이너리 포맷을 여러 언어에 맞춰 손으로 구현/유지하는 비용이 이 프로젝트 스코프 대비 과도하다고 판단해 Protobuf를 유지하기로 했다.

## 5. 파생 결정: 바디 객체의 소유권 (Arena → `unique_ptr`)

바디가 항상 새로 생성되는 객체라면, 그 객체를 누가 얼마나 들고 있을지 정해야 한다. 처음엔 `google::protobuf::Arena`로 일괄 할당/해제하는 방식을 검토했으나, `Packet`이 큐(`PacketDispatcherBase`의 `LockQueue<Packet>`)에 실려 다른 스레드에서 나중에 처리될 수 있는 구조라 **Arena의 수명을 누가 책임지는지가 불명확**했다 — `Packet`이 살아있는 동안 그 `Packet`을 만든 Arena도 같이 살아있어야 하는데, 이를 보장하는 코드가 없었다.

세 가지 대안을 비교했다:

1. `Packet : PacketHeader` 상속 + 템플릿 — `PacketHeader`는 `#pragma pack(1)`로 와이어 바이트에 그대로 얹혀 쓰이는 POD 구조체(`PacketSerializer.h:26`)라, 다형성(가상 소멸자)을 섞으면 `sizeof`가 늘어나 프로토콜 자체가 깨진다. 기각.
2. `Packet<T> : IPacket` 템플릿 + 값 보유 — 서로 다른 `T`가 서로 다른 타입이 되어 하나의 큐에 이종 타입을 담을 수 없고, 결국 `Packet<T>` 전체를 힙에 올려야 해서 문제가 해결되지 않고 자리만 옮겨감. 게다가 `T`마다 크기가 달라 나중에 메모리 풀을 적용하기도 더 까다로워짐. 기각.
3. **`Packet`은 단일 concrete 타입 유지, `m_body`를 `std::unique_ptr<PacketBody>`로 보유** — `Packet` 자체는 지금처럼 `LockQueue<Packet>`에 값으로 그대로 들어가고, 소유권이 필요한 대상(바디)만 `unique_ptr`로 명시. 채택.

## 6. 관련 코드 위치

- `02_ServerEngine/Include/EngineCommon/Packet.h:14-50` — `Packet` 클래스, `m_body`를 `std::unique_ptr<PacketBody>`로 보유
- `02_ServerEngine/Source/EngineCommon/Packet.cpp` — 생성자/이동 시맨틱스
- `02_ServerEngine/Include/EngineCommon/PacketSerializer.h:9-39` — `Write()`: 바디를 링버퍼에 직접 `SerializeToArray`
- `02_ServerEngine/Include/EngineCommon/PacketSerializer.h:58-91` — `Read()`: 바디를 `ParseFromArray`로 역직렬화, `unique_ptr`로 반환
- `02_ServerEngine/Include/EngineCommon/PacketSerializer.h:41-56` — `PeekHeader()`: 헤더만 Zero-Copy로 확인
- `01_ServerBase/Include/DataStruct/RingBuffer.h` — 네트워크 계층 Zero-Copy의 근거
