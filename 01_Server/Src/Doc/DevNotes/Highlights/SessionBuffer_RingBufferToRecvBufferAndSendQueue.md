# 세션 버퍼 설계: tail-skip 링버퍼를 수신 선형 버퍼 + 송신 큐로 교체한 이유

## 1. 배경

세션의 송수신 버퍼는 원래 둘 다 `RingBuffer`였다. 이 링버퍼는 예약을 항상 **연속된 한 구간**으로만 돌려주고, 꼬리 공간이 모자라면 그 경계를 기록한 뒤 앞(index 0)에서 통째로 쓰는 **tail-skip 방식**이다. 덕분에 패킷을 포인터 캐스팅으로 바로 읽고 쓸 수 있다.

수신 루프를 붙이기 전에 버퍼를 점검하다가, 이 구조가 수신 쪽에서는 성립하지 않는다는 것을 확인했다.

## 2. 문제

### 2.1 수신: 버퍼 끝에 걸친 패킷을 영원히 읽지 못한다

tail-skip이 연속성을 보장하는 조건은 **"한 번의 쓰기 = 완결된 메시지 1개"** 다. 송신은 패킷 크기를 알고 통째로 예약하므로 성립한다. 수신은 TCP 스트림이 패킷 경계와 무관하게 도착하므로 성립하지 않는다.

```
 (버퍼 크기 10, 패킷 크기 6 가정)

 패킷 B의 앞 4바이트가 끝에, 나머지 2바이트가 앞에 저장됨
 인덱스:  0   1   2   3   4   5   6   7   8   9
        ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
        │ B │ B │   │   │   │   │ B │ B │ B │ B │
        └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘
                  ▲ W             ▲ R

 CreateReader(6) → 연속 구간은 [6..9] 4바이트뿐 → 4바이트로 clamp
 PacketSerializer::Read는 "요청 크기 미만이면 실패" → 데이터를 더 받아도 계속 실패
```

- `CreateAllWriter`는 tail-skip 없이 물리 끝까지 예약하고(`RingBuffer.cpp:116`), `CommitWrite`가 커서를 0으로 넘긴다(`RingBuffer.cpp:193`).
- `CreateReader`는 연속 구간만 반환한다(`RingBuffer.cpp:84-93`).

재현 프로그램(100바이트 패킷을 계속 수신하고 절반 속도로 소비)에서 163개를 처리한 뒤 "요청 100, 확보 84"로 멈췄고, 이후 재시도는 모두 실패했다. 기존 `RingBufferTest.NetIO`는 패킷 1개만 왕복해서 이 경우를 잡지 못했다.

### 2.2 송신: 구조적으로 맞지 않는 점

송신은 걸침 문제는 없지만 다른 한계가 있었다.

- **스레드 경계**: 로직 스레드가 쓰고 I/O 스레드가 읽는데, `RingBuffer`는 커서가 atomic이 아니고 읽기 커밋의 `Clear()`가 쓰기 커서까지 건드린다(`RingBuffer.cpp:219-222`). 락 없이는 쓸 수 없다.
- **브로드캐스트**: 같은 패킷을 주변 세션 N개에 보내려면 세션마다 자기 링버퍼에 직렬화해야 한다.
- **가득 참 오판정**: 비어 있지만 커서가 중간인 상태에서 tail-skip으로 정확히 그 위치만큼 쓰면 `m_isFull`이 잘못 켜진다(`RingBuffer.cpp:42-47`, `:194`).

## 3. 선택

송수신의 쓰기 패턴이 다르므로 버퍼도 나눴다.

| | 쓰기 패턴 | 선택 |
|---|---|---|
| 수신 | 패킷 경계와 무관한 바이트 스트림 | `RecvBuffer`: 선형 버퍼 + 앞으로 당기기 |
| 송신 | 완결된 패킷 단위, 여러 세션에 같은 내용 | `SendBuffer`를 `shared_ptr`로 큐에 적재 |

### 3.1 수신: `RecvBuffer`

데이터는 항상 `[m_readPos, m_writePos)` 한 덩어리다. 완성된 패킷을 모두 처리한 뒤 `Compact()`를 호출하면, 데이터가 없을 때는 위치만 리셋하고, 뒤 공간이 최대 패킷 크기보다 작을 때만 남은 조각을 `memmove`로 앞에 당긴다. 버퍼 크기를 최대 패킷 크기의 2배로 잡아서, 당긴 직후에는 항상 최대 패킷 하나가 들어갈 공간이 남는다.

### 3.2 송신: `SendBuffer` + 세션별 큐

- `PacketSerializer::Serialize`가 패킷 1개를 `SendBuffer` 1개로 만들고 `std::shared_ptr<const SendBuffer>`로 반환한다. `const`로만 공유하므로 직렬화 이후에는 불변이다.
- 세션은 뮤텍스로 보호하는 큐와 "전송 중" 플래그를 갖고, `async_write`를 세션당 하나만 건다. 전송 중에 쌓인 패킷은 다음 번에 한 번의 `async_write`로 묶어 보낸다.
- `async_write`가 진행 중인 버퍼는 세션의 `m_sendingBuffers`가 붙잡고 있어서, 호출자가 버퍼를 놓아도 전송이 끝날 때까지 살아 있다.
- 앱 계층은 `ISession::Send`와 `SendBuffer`만 알고, Boost는 `BoostSession` 안에만 있다.

## 4. 트레이드오프

### 4.1 수신 버퍼 대안

| 방식 | 장점 | 단점 | 결정 |
|---|---|---|---|
| 선형 버퍼 + 앞으로 당기기 | 패킷이 항상 연속, 구현이 짧음 | 뒤 공간이 모자랄 때 `memmove` (미완성 패킷 1개 분량) | **채택** |
| 링버퍼 유지, 걸친 경우만 임시 버퍼로 복사 | 버퍼 구조 유지 | "모든 접근은 Zero-Copy"라는 Reader 계약이 깨지고 분기와 임시 저장소가 생김 | 기각 |
| Scatter-Gather(두 조각 반환) | 버퍼 내 복사 없음 | protobuf `ParseFromArray`가 연속 메모리를 요구해서 결국 복사하거나 `ZeroCopyInputStream`을 구현해야 함 | 기각 |
| mmap 이중 매핑 | 항상 연속, 복사 없음 | Linux 전용 코드, 용량이 페이지 단위, 세션 수만큼 매핑. 이 프로젝트 스코프 대비 공수가 큼 | 기각 |

### 4.2 송신 버퍼 대안

| 방식 | 장점 | 단점 | 결정 |
|---|---|---|---|
| 패킷별 버퍼를 큐에 적재 | 한 번 직렬화한 버퍼를 여러 세션이 공유, `async_write` 수명 관리가 `shared_ptr`로 해결됨 | 패킷마다 힙 할당, 큐 상한을 직접 둬야 함 | **채택** |
| `RingBuffer` 유지 | 할당 없음, 고정 크기라 자연히 상한이 있음 | 락 필요, 브로드캐스트 시 세션마다 직렬화, 오판정 버그 수정 필요 | 기각 |
| 수신과 같은 선형 버퍼 | 클래스 하나로 통일 | `async_write` 진행 중에는 데이터를 옮길 수 없어 버퍼 두 개를 번갈아 써야 함 | 기각 |

### 4.3 송신 큐의 동시성

| 방식 | 장점 | 단점 | 결정 |
|---|---|---|---|
| 뮤텍스 + 큐 | 생산자가 하나든 여럿이든 그대로 동작. 락 구간이 포인터 하나 넣고 빼기 | 패킷마다 락 1회 | **채택** |
| 락프리 SPSC 큐 | 락 없음 | 생산자가 하나라는 전제가 깨지면 버그. "전송 중" 플래그와의 조합을 따로 풀어야 함 | 기각 |

소켓은 세션별 strand를 executor로 써서, I/O 스레드가 여러 개여도 한 세션의 소켓 작업이 겹치지 않게 했다. `Send`는 소켓을 직접 건드리지 않고 strand로 `post`한다.

### 4.4 감수한 것

- **패킷당 할당**: 풀링으로 줄일 수 있지만 측정하지 않았다. 할당 비용이 문제로 확인되면 그때 적용한다.
- **최대 패킷 크기 제한**: `PacketHeader::kMaxPacketSize`(8KB)를 넘는 패킷은 직렬화 단계에서 거부한다. 수신 버퍼 크기와 묶여 있는 값이다.
- **큐 상한**: 1024개를 넘으면 `Send`가 실패하고 연결을 끊는다. 링버퍼가 주던 자연스러운 상한을 정책으로 대신한 것이다.

## 5. 검증

- `RecvBufferTest.StreamAcrossBufferEnd_AlwaysContiguous`: 패킷 1000개를 1~7바이트씩 쪼개 넣어 버퍼 끝을 반복해서 지나도 모든 패킷이 연속으로 읽힌다.
- `PacketSerializerTest.OneByteAtATime` / `ManyPacketsAcrossBufferEnd`: 1바이트씩 와도, 여러 패킷이 붙어 와도 올바르게 파싱한다.
- `BoostSessionSendTest`: 루프백 소켓으로 5000개 순서 보장, 생산자 4개 동시 전송 시 유실 없음과 생산자별 순서 보장, 버퍼 하나를 세션 8개에 브로드캐스트, 큐 초과 시 연결 종료.

## 6. 남은 것

- 수신 루프(`async_read_some` → 파싱 → 디스패처 전달)는 디스패처 완성(Todo P0-4) 이후에 붙인다.
- `RingBuffer`는 코드에 남겨 두었고 현재 세션에서는 쓰지 않는다. 2.2의 오판정 버그는 수정하지 않았다.

## 7. 관련 코드 위치

- `01_ServerBase/Source/DataStruct/RecvBuffer.cpp:37-54` — `Compact()`: 리셋 / 유지 / 당기기 규칙
- `01_ServerBase/Source/DataStruct/RecvBuffer.cpp:4-13` — 생성자: `minWritableSize <= capacity / 2` 검증
- `01_ServerBase/Include/DataStruct/SendBuffer.h` — 불변으로 공유하는 송신 버퍼
- `02_ServerEngine/Source/EngineCommon/PacketSerializer.cpp:3-29` — `Serialize()`: 패킷 1개를 `SendBuffer` 1개로
- `02_ServerEngine/Source/EngineCommon/PacketSerializer.cpp:31-64` — `PeekHeader()` / `IsValidHeader()` / `Read()`
- `02_ServerEngine/Include/EngineInterface/ISession.h:18` — 전송 Port
- `02_ServerEngine/Source/Boost/BoostSession.cpp:5` — 소켓 executor를 세션별 strand로 생성
- `02_ServerEngine/Source/Boost/BoostSession.cpp:27-64` — `Send()`: 큐 적재, 상한 검사, strand로 `post`
- `02_ServerEngine/Source/Boost/BoostSession.cpp:66-90` — `FlushSendQueue()`: 큐를 통째로 꺼내 묶음 전송
- `02_ServerEngine/Source/Boost/BoostSession.cpp:92-109` — `CompleteSend()`: 버퍼 해제, 에러 처리, 다음 전송
- `01_ServerBase/Source/DataStruct/RingBuffer.cpp:84-93`, `:116`, `:193` — 수신 걸침 문제의 원인 지점