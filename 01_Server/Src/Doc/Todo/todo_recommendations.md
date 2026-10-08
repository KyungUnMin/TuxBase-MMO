# TuxBase-MMO 작업 로드맵 (상세)

> 기준: 2026-09-21, 커밋 `07d251f`.
> 갱신: 2026-10-08, 커밋 `903c88a` — 세션 버퍼를 `RingBuffer`에서 수신 `RecvBuffer` + 송신 큐로 교체하고 P0-2를 완료했다. 배경은 `DevNotes/Highlights/SessionBuffer_RingBufferToRecvBufferAndSendQueue.md` 참고.
> 이전 버전은 Acceptor·Session·직렬화 등을 미구현으로 적고 있어 현재 코드와 어긋났기 때문에 전면 교체했다.
> 단순 체크리스트는 `TodoList.txt`에 있다. 이 문서는 각 항목의 **근거, 완료 조건, 설계 트레이드오프**를 담는다.

---

## 1. 원칙

- 이 프로젝트는 **게임 서버 개발자 포트폴리오**다. 분산/이중화, 상용 모니터링, 배포 파이프라인은 다루지 않는다.
- 목표는 "엔진 + 최소 게임 컨텐츠 + 실측 수치"로 완결된 포트폴리오를 만드는 것이다.
- 모든 작업에 **완료 조건(DoD)** 을 적는다. DoD를 만족하지 못하면 다음 단계로 넘어가지 않는다.
- 설계가 갈리는 지점은 5장에서 트레이드오프를 비교하고 결정한다.
- 성능 관련 주장은 벤치마크 수치를 함께 남긴다 (CLAUDE.md 문서화 규칙).

---

## 2. 현재 상태 (2026-10-08)

| 영역 | 있음 | 없음 |
|---|---|---|
| 기반(01_ServerBase) | `RecvBuffer`(수신용 선형 버퍼), `SendBuffer`(송신용 불변 버퍼), RingBuffer(Reader/Writer RAII, 현재 세션에서는 미사용)와 테스트, LockQueue, LockStack, Thread, Logger, SerialGenerator, Assert/BackTrace/SignalHandler | Logger의 엔진 연동 |
| 엔진(02_ServerEngine) | `BoostNetEngine`(io_context, 스레드, 세션 풀), 서버 accept, 클라이언트 connect + 재시도, `PacketSerializer`, `Packet`, `ISession::Send` Port, 세션 송신 큐(`async_write` 체인, 세션별 strand) | 세션 수신 루프, 세션 반납, 세션 ID 조회, 디스패처 구현 |
| 앱(03_ServerApp) | 엔진 기동 | 컨텐츠 스레드, 핸들러 |
| 더미 클라이언트(04_DummyClient) | 1024세션 접속 (5초 후 종료) | 패킷 송수신, 시나리오 |
| 프로토콜(03_Share) | `C2S_Login`, `S2C_LoginResponse` | 이동/입장 등 컨텐츠 패킷 |
| 테스트(99_Test) | RingBuffer, RecvBuffer, StopWatch, 직렬화(`PacketSerializer`), 세션 송신(루프백 소켓) | 세션 수신, 서버-클라이언트 통합 테스트 |

**검증된 사실**: 2026-09-20에 서버/더미클라이언트를 별도 컨테이너로 띄워 연결 수립을 확인했다.
`mainserver` DNS 해석, `0.0.0.0:13000` LISTEN, 서버·클라이언트 양쪽 ESTABLISHED 1018개가 일치했다.
컨테이너 간 통신 자체는 정상이며, 이 로드맵은 그 위에서 **패킷 계층**을 채우는 작업이다.

---

## 3. 코드에서 발견한 결함

| # | 결함 | 위치 | 영향 | 처리 |
|---|---|---|---|---|
| 1 | `class INetEngine; class ISession;`가 클래스 **안**에 있어 전역 타입이 아닌 중첩 클래스를 선언함 | `PacketDispatcherBase.h:8-9` | `Dispatch(ISession&, …)`가 실제 `ISession`과 다른 타입이라 연결 불가 | P0-4 |
| 2 | 기본 생성자만 `delete`이고 다른 생성자가 없음 | `PacketDispatcherBase.h:12` | 파생 클래스를 생성할 수 없음 | P0-4 |
| 3 | `EnqueuePacket` 선언만 있고 정의가 없음 (`.cpp`는 include 한 줄). 인자도 세션에서 더 이상 쓰지 않는 `RingBufferReader&&`임 | `PacketDispatcherBase.h:20`, `PacketDispatcherBase.cpp:1` | 호출 시 링크 에러. 수신 버퍼 메모리를 다른 스레드로 넘기는 형태라, I/O 스레드에서 파싱을 끝낸 `Packet`을 받도록 바꿔야 함 | P0-4 |
| 4 | 패킷 ID로 어떤 Protobuf 타입을 만들지 알 방법이 없음 | `PacketDispatcherBase.h:20`, `Packet.h` | 수신 바디를 `PacketBody`로 만들 수 없음 | P0-4 |
| 5 | ~~`ISession`이 빈 인터페이스라 상위 계층이 응답을 보낼 수 없음~~ | `ISession.h:18` | **해결(2026-10-08)**: `ISession::Send(shared_ptr<const SendBuffer>)` 추가 | 완료 |
| 6 | ~~`SendPacket`이 송신 버퍼에 쓰기만 하고 전송을 시작하지 않음~~ | `BoostSession.cpp:27-109` | **해결(2026-10-08)**: `SendPacket` 제거, 송신 큐 + `async_write` 체인으로 교체 | 완료 |
| 7 | `Start()`가 소켓 옵션만 설정하고 `async_read`가 없음 | `BoostSession.cpp:12-18` | 수신 불가, 종료 감지 불가 | P0-1 |
| 8 | 정상 연결의 종료·반납 경로가 없음. `PushSession`은 accept/connect 실패와 엔진 종료에서만 호출됨 | `BoostNetEngineServer.cpp:64,70`, `BoostNetEngineClient.cpp:53,61` | 실측: 클라이언트 1회 실행(세션 1024개) 후 서버에 `CLOSE_WAIT` 1019개가 남음. 서버 풀도 1024개라 재접속 시 accept 가능한 세션이 급감함 | P0-3 |
| 9 | `FindSession`이 `nullptr`을 반환하는 stub | `BoostNetEngine.h:24-28` | 세션 ID로 세션을 찾을 수 없음 | P0-3 |
| 10 | `std::cout` 출력과 `TODO : LOG_ERROR` 주석이 남아 있음 | `BoostSession.cpp:17`, `BoostNetEngineClient.cpp:65,82`, `BoostNetEngineServer.cpp:52,73`, `PacketSerializer.cpp:10,25`(`TODO : Error Log`) | 오류가 조용히 사라지고 서버 쪽 관측 수단이 없음 | P1-7 |
| 11 | `SignalHandler`가 종료 플래그를 세팅하지만 `main`이 이를 보지 않고 `sleep_for(100000s)`로 대기 | `SignalHandler_linux.cpp:19-30`, `03_ServerApp/main.cpp:10` | Ctrl+C/`docker stop`으로 `Stop()` 경로를 타지 못함 | P1-8 |
| 12 | `RingBuffer` 가득 참 오판정: 비어 있지만 커서가 중간인 상태에서 tail-skip으로 정확히 그 위치만큼 쓰면 `m_isFull`이 켜짐 | `RingBuffer.cpp:42-47`, `:194` | 다음 읽기 전까지 쓰기 예약이 거부됨 (데이터 손상 없음) | 보류 (현재 `RingBuffer`를 쓰는 곳이 없음. 다시 쓸 때 수정) |
| 13 | `RingBuffer`에서 `CreateAllWriter`로 쌓은 스트림을 `CreateReader(패킷 크기)`로 읽으면 버퍼 끝에 걸친 패킷을 영원히 읽지 못함 | `RingBuffer.cpp:84-93`, `:116`, `:193` | 수신 버퍼로 쓰면 세션 수신이 영구 정지 | 우회 완료 (세션 수신은 `RecvBuffer` 사용). `RingBuffer` 자체는 보류 |

---

## 4. 로드맵

### P0. 패킷이 오가는 수직 슬라이스 (최우선)

목표: **DummyClient가 `C2S_Login`을 보내고, 서버가 `S2C_LoginResponse`로 응답하는 왕복**을 통합 테스트로 고정한다.
README에 적은 "I/O 스레드 → LockQueue → 컨텐츠 스레드" 구조가 실제로 동작하게 되는 단계다.

#### P0-1. 세션 수신 루프
- **작업**: `Start()` 이후 `async_read_some`(`RecvBuffer::GetWritableSpan`) → `CommitWrite` → `PacketSerializer::PeekHeader`/`IsValidHeader`로 완성 여부 판별 → `Read`로 파싱 → 디스패처로 전달 → `Compact` → 다시 `async_read_some`. 수신 핸들러는 세션 strand 위에서 실행되므로 `m_recvBuffer`에 락을 걸지 않는다.
- **선행 완료(2026-10-08)**: 수신 버퍼를 `RecvBuffer`(선형 버퍼 + 앞으로 당기기)로 교체했다. tail-skip `RingBuffer`는 버퍼 끝에 걸친 패킷을 읽지 못해 수신에 쓸 수 없었다(결함 13). 사용 패턴은 `ClassExplanation/01_ServerBase/DataStruct/RecvBuffer.md` 4장 참고.
- **DoD**: 패킷을 1바이트씩 쪼개 보내도, 여러 패킷을 한 번에 붙여 보내도 모두 올바르게 파싱한다는 테스트 (TCP 스트림 특성 검증). 버퍼·직렬화 단위로는 통과했다(`PacketSerializerTest.OneByteAtATime`, `ManyPacketsAcrossBufferEnd`). 남은 것은 **실제 소켓 수신 루프를 거치는** 테스트다.
- **의존**: 파싱한 패킷을 넘길 디스패처(P0-4)가 필요하다.
- **코드**: `BoostSession.cpp:12-18`, `BoostSession.h:42`, `PacketSerializer.cpp:31-64`, `RecvBuffer.cpp`
- **포트폴리오 포인트**: TCP 스트림의 패킷 경계 처리. README의 RingBuffer 설명은 현재 구현과 다르다 (D-3).

#### P0-2. 세션 송신 경로 + `ISession` 전송 Port — 완료 (2026-10-08)
- **구현 결과**: `PacketSerializer::Serialize`가 패킷 1개를 `SendBuffer` 1개로 만들고, `ISession::Send(std::shared_ptr<const SendBuffer>)`로 세션 송신 큐에 넣는다. 아래 원래 계획의 `Send(packetId, body)` 시그니처 대신, 한 번 직렬화한 버퍼를 여러 세션이 공유할 수 있는 형태를 택했다. 상세는 `ClassExplanation/02_ServerEngine/Boost/BoostSession_SendQueue.md` 참고.
- **검증**: `BoostSessionSendTest`(루프백 소켓) — 순서 보장, 다중 생산자, 브로드캐스트, 큐 초과 시 연결 종료. 실제 서버-더미클라이언트 간 수신 확인은 P0-5에서 한다.
- **작업(원래 계획)**:
  - `SendPacket` 이후 송신을 시작한다. `async_write`는 세션당 **동시에 하나만** 걸고(in-flight 플래그), 완료 핸들러에서 남은 데이터를 이어 보낸다.
  - `ISession`에 `virtual bool Send(UINT16 packetId, const PacketBody& body) = 0` 같은 전송 Port를 추가한다. 템플릿은 가상 함수가 될 수 없어서 `PacketBody`(protobuf `Message`) 참조를 받는다.
  - 컨텐츠 스레드(쓰기)와 I/O 스레드(읽기)의 경계는 5장 결정 D를 따른다.
- **DoD**: 서버가 보낸 패킷을 클라이언트가 수신한다. 앱 계층 코드가 `Boost/` 헤더를 include하지 않는다.
- **코드**: `BoostSession.cpp:27-109`, `ISession.h:18`, `PacketSerializer.cpp:3-29`, `SendBuffer.h`
- **포트폴리오 포인트**: Clean Architecture Port 확장. 안쪽 계층이 바깥 구현체를 모르는 상태 유지.

#### P0-3. 세션 수명 관리 + 세션 ID 조회
- **작업**:
  - 소켓 종료 감지(read 에러/EOF) → 소켓 닫기 → 버퍼 초기화 → 풀 반납.
  - 접속 시 세션 ID 부여(`SerialGenerator` 활용), `FindSession(sessionId)` 구현.
  - 풀 반납 시 세션 상태를 재사용 가능하게 초기화한다. 송신 큐(`m_sendQueue`, `m_sendingBuffers`, `m_isSending`)와 수신 버퍼(`RecvBuffer::Clear`)를 포함한다.
  - 소켓을 닫는 동작은 세션 strand로 넘긴다. 현재 `BoostNetEngineClient::RetryConnect`가 `CloseSocket()`을 strand 밖에서 직접 호출한다(`BoostNetEngineClient.cpp:86`). 연결 전이라 지금은 겹칠 작업이 없지만, 종료 경로를 만들 때 함께 정리한다 (`LearnWithAI/Asio_Strand.md` 주의할 점 참고).
- **DoD**: 더미 클라이언트를 여러 번 실행해도 서버 세션 풀이 매번 복원된다. 클라이언트 종료 후 서버에 `CLOSE_WAIT`이 남지 않는다.
- **코드**: `BoostNetEngineServer.cpp:56-77`, `BoostNetEngine.h:24-28`, `SerialGenerator.h`
- **포트폴리오 포인트**: 스마트 포인터 기반 소유권 설계. 5장 결정 A 참고.

#### P0-4. 디스패처 완성 + 메시지 팩토리
- **작업**:
  - 결함 1~4 수정(전방 선언 위치, 생성자, `EnqueuePacket` 구현). `EnqueuePacket`은 `RingBufferReader&&` 대신 파싱이 끝난 `Packet&&`을 받도록 바꾼다.
  - 패킷 ID → Protobuf 메시지 생성 방식을 정한다 (5장 결정 E).
  - `PacketId → 핸들러` 등록 구조를 만든다 (5장 결정 C).
  - 컨텐츠 스레드 루프: `LockQueue<Packet>`에서 꺼내 `Dispatch` 호출 (5장 결정 B).
- **DoD**: 수신 패킷이 I/O 스레드 → `LockQueue` → 컨텐츠 스레드 → 핸들러 순으로 전달된다. 핸들러 등록 코드는 특정 패킷 타입을 컴파일 타임에 검증한다.
- **코드**: `PacketDispatcherBase.h`, `PacketDispatcherBase.cpp`, `Packet.h`
- **포트폴리오 포인트**: Factory/Command 패턴의 실질적 필요가 있는 지점(다형성 + 확장 지점). 도입 근거를 `Highlights/`에 정리한다.

#### P0-5. 첫 E2E: 로그인 왕복
- **작업**: DummyClient가 접속 후 `C2S_Login`을 보내고 `S2C_LoginResponse`를 받으면 성공을 출력한다. 서버 핸들러는 임시로 "항상 성공"을 반환해도 된다.
- **DoD**: 서버와 클라이언트를 프로세스로 띄우는 통합 테스트(또는 스크립트)가 통과한다. DummyClient가 5초 후 무조건 종료하는 현재 구조(`04_DummyClient/main.cpp:9`)를 시나리오 완료 기준 종료로 바꾼다.
- **코드**: `04_DummyClient/Source/main.cpp`, `03_ServerApp/Source/main.cpp`, `Example.proto`

#### P0-6. 입력 방어
- **작업**: `m_size`가 헤더 크기 미만이거나 상한 초과일 때, 알 수 없는 패킷 ID일 때, 파싱에 실패할 때 해당 연결을 종료한다.
- **선행 완료(2026-10-08)**: 크기 상한 `PacketHeader::kMaxPacketSize`(8KB)와 `PacketSerializer::IsValidHeader`가 있다. 남은 것은 수신 루프(P0-1)에서 검증 실패 시 연결을 끊는 처리와, 알 수 없는 패킷 ID 처리(P0-4)다.
- **DoD**: 잘못된 헤더/ID/바디를 보내는 테스트 클라이언트로 서버가 죽지 않고 해당 세션만 끊는 것을 확인한다.
- **코드**: `PacketSerializer.cpp:43-64`, `Packet.h:10,40-47`
- **포트폴리오 포인트**: 서버 개발자 면접에서 자주 다뤄지는 "신뢰할 수 없는 입력" 처리.

---

### P1. 엔진 마감 (P0 직후, 작은 단위)

#### P1-7. 로깅 연동
- `std::cout`(`BoostSession.cpp:17`)과 `TODO : LOG_ERROR` / `TODO : Error Log` 주석(`BoostSession.cpp`, `BoostNetEngineClient.cpp`, `BoostNetEngineServer.cpp`, `PacketSerializer.cpp`)을 `Logger`로 교체한다.
- **DoD**: 접속/종료/오류가 서버 로그에 남는다. 엔진 코드에 `std::cout`이 없다.

#### P1-8. Graceful shutdown
- `SignalHandler::IsShutdownRequested()`를 `main`에서 폴링해 종료 시 `Stop()`이 호출되게 한다. `SignalHandler`는 이미 구현되어 있으므로 **연결만** 하면 된다.
- **DoD**: `docker stop`/Ctrl+C 시 `Stop()` 경로를 타고 종료된다.
- **코드**: `03_ServerApp/main.cpp:10`, `SignalHandler_linux.cpp`

#### P1-9. 하트비트/타임아웃
- `BoostSession.cpp:15`에 "keep_alive는 하트비트로 대체"라고 적어 둔 결정을 실제로 구현한다. 일정 시간 패킷이 없으면 세션을 정리한다.
- **DoD**: 응답 없는 클라이언트가 타임아웃 후 세션이 반납된다.

#### P1-10. 실행 인자/환경변수
- 포트, 스레드 수, 접속 대상 호스트를 인자 또는 환경변수로 받는다. JSON/TOML 설정 시스템은 과하므로 하지 않는다.
- **DoD**: 코드 수정 없이 포트/호스트를 바꿀 수 있다 (`03_ServerApp/main.cpp:8`, `04_DummyClient/main.cpp:7`의 하드코딩 제거).

---

### P2. 최소 게임 컨텐츠 (차별화 요소)

#### P2-11. 간이 로그인 + 캐릭터 입장
- 인메모리 계정으로 로그인하고 월드에 입장한다. DB는 사용하지 않는다 (DB/로그인 서버는 후속 Node.js 서버 착수 시).
- **DoD**: 두 클라이언트가 각자 로그인해 월드에 입장한다.

#### P2-12. 이동 동기화 + 주변 브로드캐스트
- 이동 패킷을 받아 좌표를 갱신하고 주변 플레이어에게 전달한다.
- **DoD**: 클라이언트 A의 이동이 클라이언트 B에 수신된다.

#### P2-13. 월드 틱 루프
- 고정 타임스텝(예: N ms)으로 월드를 갱신한다. 타이머는 여기서 처음 필요해진다.
- **DoD**: 틱 지연이 로그로 측정된다.

#### P2-14. AOI (그리드 기반 관심 영역)
- 전체 브로드캐스트를 셀 단위 관심 영역 브로드캐스트로 바꾼다.
- **DoD**: 관심 영역 밖 플레이어에게는 이동 패킷이 가지 않는다.

#### P2-15. 봇 부하 테스트 + 벤치마크
- DummyClient를 N개 봇 시나리오로 확장하고 **실측 수치**를 문서에 남긴다 (동시 접속 수, 초당 패킷 수, 틱 지연 등).
- **DoD**: 측정 환경(컨테이너 사양, 스레드 수)과 수치가 `Doc/`에 기록된다. 수치 없이 "빠르다"는 표현은 쓰지 않는다.

---

### 병행 작업 (문서)

| # | 작업 |
|---|---|
| D-1 | 새 핵심 클래스(`BoostSession`, 디스패처 등) 추가·변경 시 `DevNotes/ClassExplanation/`을 같이 갱신한다. 현재 문서: `RingBuffer.md`, `RecvBuffer.md`, `BoostSession_SendQueue.md` (뒤 두 개는 2026-10-08 작성). |
| D-2 | 결정 A/C/E 등 흥미로운 트레이드오프는 `DevNotes/Highlights/`에 정리한다 (`PacketBody_ProtobufVsZeroCopy.md` 형식 참고). 2026-10-08에 `SessionBuffer_RingBufferToRecvBufferAndSendQueue.md`(결정 D 포함)를 작성했다. |
| D-3 | README의 아키텍처 다이어그램 중 Packet Parser → LockQueue → Content Thread는 P0-1, P0-4 이전에는 구현되지 않았다. P0 완료 후 문구를 실제와 맞춘다. 또한 README가 `RingBuffer`를 세션 수신 버퍼로 설명하는데, 2026-10-08부터 수신은 `RecvBuffer`, 송신은 큐 방식이라 이 부분도 함께 고친다. |

---

## 5. 설계 결정 (트레이드오프)

### A. 세션 수명 모델

| 방식 | 장점 | 단점 |
|---|---|---|
| 풀 소유(`unique_ptr`/`optional`) + 세션은 `sessionId`로 조회 (**권장**) | 할당이 없고 구조가 단순함. 소유권이 풀 하나로 명확함. `Packet`이 이미 `sessionId`를 들고 있어 자연스러움 | 종료 후 재사용된 세션에 늦게 도착한 핸들러가 접근할 수 있음. 세대(generation) 번호가 필요 |
| `shared_ptr` + `enable_shared_from_this` | 진행 중 핸들러가 세션을 붙잡아 수명이 안전함 | 참조 카운트 비용. 풀과 병행하려면 커스텀 deleter가 필요해 복잡도 증가 |

권장 이유: 이미 `LockStack` 풀 구조이고 CLAUDE.md의 소유권 명시 원칙(소유는 스마트 포인터, 접근은 non-owning)에 맞는다. `sessionId`에 세대 번호를 포함하면 재사용 문제를 막을 수 있다.

### B. 컨텐츠 스레드 모델

| 방식 | 장점 | 단점 |
|---|---|---|
| 단일 로직 스레드 (**권장**) | 게임 로직에 락이 필요 없고 구현이 쉬움. 디버깅 용이 | 로직 처리량이 스레드 하나에 묶임 |
| 룸/존 단위 스레드 | 확장 구조를 보여줄 수 있음 | 룸 간 이동·동기화 복잡도 증가 |

P0~P1은 단일 로직 스레드로 진행한다. 필요성이 실측(P2-15)으로 확인되면 그때 룸 단위 분리를 검토한다.

### C. 핸들러 등록

| 방식 | 장점 | 단점 |
|---|---|---|
| `unordered_map<id, function>` | 구현이 간단하고 유연함 | 해시 조회 비용 (수치 미측정) |
| `std::array` 인덱싱 | 조회가 O(1) | ID가 1001부터라 오프셋/범위 관리 필요. 등록되지 않은 슬롯 처리 필요 |
| 템플릿 등록 `Register<C2S_Login>(handler)` + 내부는 맵 (**권장**) | 핸들러 인자 타입을 컴파일 타임에 검증해 캐스팅 실수를 차단함 | 템플릿 코드가 늘어남 |

성능 우열은 측정 전에는 주장하지 않는다. 지금은 안전성과 가독성 기준으로 선택한다.

### D. 송신 스레드 경계

| 방식 | 장점 | 단점 |
|---|---|---|
| `SendPacket` 호출 스레드가 송신 버퍼에 직접 쓰고 락으로 보호 | 구현이 단순함 | 브로드캐스트처럼 여러 스레드가 같은 세션에 쓰면 락 경합. 락을 잡은 채 protobuf 직렬화를 하게 될 수 있음 |
| 송신 요청을 세션의 I/O 컨텍스트에 `post`해서 I/O 스레드에서만 버퍼를 다룸 | 버퍼 접근이 한 스레드로 모여 락이 필요 없음 | 요청마다 핸들러 할당과 큐잉 비용, 직렬화 위치를 어디로 둘지 결정 필요 |

**결정(2026-10-08)**: 두 방식을 섞었다. 직렬화는 호출 스레드에서 락 없이 `SendBuffer`로 끝내고, 세션은 뮤텍스로 보호하는 큐에 `shared_ptr`만 넣는다(락 구간은 포인터 하나 넣고 빼기). 소켓을 다루는 전송 시작은 세션 strand로 `post`한다. 생산자가 하나든 여럿이든 그대로 동작하므로 결정 B가 바뀌어도 수정할 필요가 없다. 표의 `SendPacket`과 "송신 버퍼"는 교체 전 구조 기준 표현이다. 대안 비교는 `Highlights/SessionBuffer_RingBufferToRecvBufferAndSendQueue.md` 4장 참고.

### E. 패킷 ID → Protobuf 메시지 생성

| 방식 | 장점 | 단점 |
|---|---|---|
| `switch(id)` 수동 분기 | 가장 단순하고 명시적 | 패킷 추가마다 분기 수정. 누락을 컴파일러가 못 잡음 |
| 등록형 팩토리 `map<id, function<unique_ptr<PacketBody>()>>` (**권장**) | 패킷 추가가 등록 한 줄로 끝나고 핸들러 등록(결정 C)과 한 곳에서 묶을 수 있음 | 등록 누락은 런타임에 발견됨 |
| protobuf Descriptor 리플렉션(enum 이름 → 메시지 이름 규칙) | 코드 추가 없이 자동 | 이름 규칙에 의존해 깨지기 쉽고, 리플렉션 비용과 디버깅 난이도가 큼 |

팩토리 패턴을 쓰는 이유는 다형성(`PacketBody` = `google::protobuf::Message`)과 확장 지점(패킷 추가)이 실제로 있기 때문이다. 패턴 자체가 목적은 아니다. 도입 근거는 `Highlights/`에 정리한다.

---

## 6. 이전 목록 대비 정리

이전 문서(20개 항목)를 현재 코드 기준으로 재분류했다.

| 이전 항목 | 처리 | 사유 |
|---|---|---|
| 1. TCP Acceptor | 완료 | `BoostNetEngineServer`에 구현됨 |
| 2. TCP Session | 일부 완료 → P0-1/3 | 세션 클래스, 풀, 쓰기(P0-2 완료)는 있으나 읽기/수명이 없음 |
| 3. IOContext Runner | 완료 | `BoostNetEngine::Start` (스레드 N개로 `io_context.run`) |
| 4. 패킷 직렬화 | 완료 | `PacketSerializer` (수신 측 연동은 P0-1) |
| 5. 세션 매니저 | P0-3에 흡수 | 별도 `ISessionManager` 없이 풀 + `FindSession`으로 충분 |
| 6. RingBuffer 복구 | 완료 | 빌드되고 테스트 존재. 단 2026-10-08부터 세션에서는 쓰지 않고 코드만 유지 (결함 12, 13) |
| 7. Config 시스템 | 축소 → P1-10 | JSON/TOML 파서는 포트폴리오 가치 대비 공수가 큼. 인자/환경변수로 충분 |
| 8. 네임스페이스 통일 | 제외 | 코드에 `common::` 네임스페이스가 존재하지 않음 (익명 네임스페이스/별칭만 있음) |
| 9. SignalHandler | 구현됨 → P1-8 | 남은 것은 `main`과의 연결 |
| 10. 패킷 핸들러 디스패처 | P0-4 | |
| 11. CI/CD | 제외 | 실서비스 운영 관점. 포트폴리오 가치 대비 공수가 큼 |
| 12. clang-tidy/format | 제외 | `.clang-format`은 이미 있음. 추가 자동화는 우선순위 낮음 |
| 13. Logger 파일 Sink | 후순위 | 엔진 연동(P1-7)이 먼저. 파일 출력은 필요할 때 |
| 14, 15. Redis, MSSQL | 제외 | DB/로그인 서버는 후속 Node.js 서버에서 다룰 예정이며 착수 전 |
| 16. 스레드 풀 | 제외 | `Thread` 래퍼와 I/O 스레드 N개가 이미 있음. 컨텐츠 스레드는 결정 B |
| 17. 타이머/스케줄러 | 흡수 → P1-9, P2-13 | 하트비트와 월드 틱에서 처음 필요해짐 |
| 18. 오브젝트 풀 | 세션 풀은 완료, 패킷 풀은 보류 | 벤치마크(P2-15)에서 할당이 병목으로 확인되면 검토 |
| 19. Echo 서버 데모 | P0-5 | 로그인 왕복이 데모 역할을 함 |
| 20. 03_Share 공유 프로젝트 | 완료 | `03_Share/Protocol/`이 이미 존재하고 서버 빌드가 참조함 |

---

## 7. 마일스톤

| 마일스톤 | 완료 조건 |
|---|---|
| **M1. 패킷 통신** (P0) | 로그인 왕복 통합 테스트 통과, 클라이언트 반복 실행 후에도 서버 세션 풀 복원(`CLOSE_WAIT` 0개), 잘못된 입력에도 서버 유지 |
| **M2. 엔진 마감** (P1) | 로깅, Ctrl+C 종료, 하트비트 타임아웃, 인자로 설정 변경 |
| **M3. 최소 컨텐츠 + 수치** (P2) | 두 클라이언트 이동 동기화, AOI, 봇 부하 테스트 결과가 `Doc/`에 기록됨 |

## 8. 개발 환경 메모

- 서버 컨테이너(`tuxbase-mainserver`)에서는 `GameServer (gdb)`, 클라이언트 컨테이너(`tuxbase-dummyclient`)에서는 `DummyClient (gdb)` 설정을 사용한다 (`Src/.vscode/launch.json`). 서버 설정이 기본값이라 클라이언트 컨테이너에서 실수로 서버를 띄우기 쉽다.
- 현재 서버 쪽 관측 수단은 `BoostSessionStart` 출력뿐이다. P1-7 전까지는 소켓 상태(`/proc/net/tcp`)로 연결을 확인할 수 있다.
