# Boost.Asio strand

## 개요

strand는 **"여기에 넣은 핸들러는 한 번에 하나씩만 실행한다"** 를 보장하는 핸들러 큐다.
게임 서버에서 흔히 쓰는 **잡큐(JobQueue) 패턴**과 같은 발상이고, Asio가 미리 만들어 둔 것이다.

이 프로젝트에서는 **세션마다 strand를 하나씩** 둔다. 그래서 한 세션의 핸들러끼리는 겹쳐 실행되지 않고, 핸들러만 건드리는 멤버 변수에는 락을 걸지 않아도 된다.

## 왜 필요한가

`io_context` 하나를 I/O 스레드 여러 개가 돌린다 (`BoostNetEngine.cpp:25-30`). 완료 핸들러는 빈 스레드가 아무거나 가져가 실행하므로, **같은 세션의 핸들러 두 개가 서로 다른 스레드에서 동시에 실행될 수 있다.**

한 세션에는 핸들러가 두 종류 생긴다.

- 송신 완료 핸들러: 다음 패킷을 소켓에 쓴다
- 수신 완료 핸들러: 소켓에서 다음 데이터를 읽기 시작한다

둘 다 같은 `m_socket`을 건드리는데, Asio 소켓 객체는 두 스레드가 동시에 건드리면 안 된다.

```
 strand 없음
   I/O 스레드1: [송신 핸들러 ......]
   I/O 스레드2:      [수신 핸들러 ......]     ← 같은 소켓을 동시에 건드림

 strand 있음
   I/O 스레드1: [송신 핸들러 ......]
   I/O 스레드2:                    [수신 핸들러 ......]
```

## 동작 원리

strand 안에는 핸들러 큐와 "지금 실행 중인가" 표시가 있다. 실행은 strand가 직접 하지 않고 `io_context`의 I/O 스레드가 한다.

1. 핸들러가 strand에 들어온다.
2. 실행 중인 스레드가 없으면, strand가 `io_context`에 자신을 등록한다.
3. 빈 I/O 스레드 하나가 그 strand를 맡아, 큐의 핸들러를 앞에서부터 하나씩 실행한다.
4. 그 사이에 새 핸들러가 들어오면 큐 뒤에 쌓이기만 한다. 다른 I/O 스레드가 끼어들어 실행하지 않는다.
5. 큐가 비면 실행 중 표시가 꺼진다.

기억할 점:

- **특정 스레드에 묶이지 않는다.** 실행하는 스레드는 매번 달라질 수 있고, 겹치지만 않는다.
- **겹치지 않는 것은 핸들러 코드다.** 실제 네트워크 송수신은 운영체제가 처리하므로, 송신과 수신이 동시에 진행되는 것은 그대로다.
- **세션끼리는 여전히 병렬이다.** strand가 세션마다 따로라서 세션 A와 세션 B의 핸들러는 동시에 실행된다.

## 잡큐 패턴과 비교

| | 잡큐 패턴 | strand |
|---|---|---|
| 큐 단위 | 보통 룸, 존 같은 게임 객체 | 이 프로젝트에서는 세션 |
| 넣는 것 | 잡(함수 객체) | 핸들러(함수 객체) |
| 직렬화 방식 | 큐를 맡은 스레드 하나가 빌 때까지 실행 | 동일 |
| 넣는 방법 | 직접 `Push` | `post`로 직접 넣거나, 비동기 작업 완료 시 Asio가 자동으로 넣음 |
| 구현 | 직접 작성 | Asio 제공 |

## 세션 코드에서 사용한 방식

### 1. 소켓을 만들 때 strand를 연결한다

```cpp
// BoostSession.cpp:5
m_socket(boost::asio::make_strand(ioContext))
```

| 코드 조각 | 의미 |
|---|---|
| `ioContext` | 핸들러를 실행하는 I/O 스레드들의 공용 대기열 |
| `make_strand(ioContext)` | 그 위에 "하나씩만 실행"하는 전용 큐를 하나 만듦 |
| `m_socket(그 큐)` | 이 소켓의 완료 핸들러는 전부 그 큐에 넣으라고 지정 |

세션 생성자가 세션마다 한 번씩 호출되므로 세션마다 strand가 하나씩 생긴다.

### 2. strand에 핸들러가 들어가는 경로는 두 가지다

**Asio가 자동으로 넣는 경우** — `m_socket`에 건 비동기 작업이 끝났을 때

```cpp
// BoostSession.cpp:86-89
boost::asio::async_write(m_socket, buffers, [this](const ErrorCode& errorCode, std::size_t /*sendSize*/)
{
    this->CompleteSend(errorCode);   // 세션 strand 위에서 실행됨
});
```

**직접 넣는 경우** — strand 밖의 스레드가 소켓 작업을 부탁할 때

```cpp
// BoostSession.cpp:56-62
boost::asio::post(m_socket.get_executor(), [this]()   // get_executor() = 세션 strand
{
    this->FlushSendQueue();
});
```

`Send()`는 로직 스레드에서 호출되는 평범한 함수 호출이라 strand를 거치지 않는다. 그래서 `Send()` 안에서는 소켓을 직접 쓰지 않고 `post`로 넘긴다. 큐 초과 시 소켓을 닫는 것도 같은 방식이다 (`BoostSession.cpp:49-53`).

### 3. 패킷 하나를 보낼 때 strand에 들어가는 것

```
 ① 로직 스레드: Send() → 패킷을 m_sendQueue에 넣고 post
        세션 strand: [FlushSendQueue]

 ② I/O 스레드가 꺼내 실행 → 안에서 async_write 시작
        세션 strand: (비어 있음)

 ③ 전송 완료 → Asio가 자동으로 넣음
        세션 strand: [CompleteSend]

 ④ I/O 스레드가 꺼내 실행 → 보낼 것이 더 있으면 다시 async_write
```

### 4. 멤버 변수별 보호 방법

세션에는 큐가 두 개 있다. strand는 **실행할 코드**를 담고, `m_sendQueue`는 **보낼 패킷**을 담는다.

| 멤버 | 누가 건드리나 | 보호 |
|---|---|---|
| `m_socket` | 핸들러만 | strand (락 불필요) |
| `m_sendingBuffers` | 핸들러만 | strand (락 불필요) |
| `m_recvBuffer` | 핸들러만 (수신 루프 구현 후) | strand (락 불필요) |
| `m_sendQueue`, `m_isSending` | 핸들러 + `Send()`를 호출하는 스레드 | `m_sendMutex` |

**strand의 장점은 "핸들러만 건드리는 멤버"에 락이 필요 없다는 것이다.** strand 밖의 스레드도 함께 건드리는 멤버는 여전히 락이 필요하다.

## 주의할 점

- **strand는 소켓을 잠그지 않는다.** strand를 거쳐 실행되는 코드끼리만 겹치지 않는다. 다른 스레드가 `post` 없이 소켓 함수를 직접 호출하면 보호되지 않고, 컴파일러도 잡아 주지 않는다.
- **세션에 소켓을 다루는 함수를 추가할 때는 "이 함수가 strand 위에서 불리는가"를 확인한다.** 완료 핸들러 안에서 불리거나, 아니면 `post`로 넘겨야 한다.
- **현재 규칙 밖에 있는 호출**: `BoostNetEngineClient::RetryConnect`가 타이머 핸들러에서 `session->CloseSocket()`을 직접 호출한다 (`BoostNetEngineClient.cpp:86`). 지금은 연결 전이라 겹칠 작업이 없다. 세션 종료 처리(Todo P0-3)를 만들 때는 닫는 동작도 strand로 넘겨야 한다.
- **핸들러 안에서 오래 걸리는 일을 하면 그 세션의 다음 핸들러가 밀린다.** 다른 세션에는 영향이 없다.
- **현재 상태**: 수신 루프가 아직 없어서 송신 핸들러만 있다. strand가 지금 막아 주는 것은 큐 초과로 소켓을 닫는 작업과 송신 핸들러가 겹치는 경우 정도이고, 주된 목적은 수신 루프(Todo P0-1)가 붙었을 때 송신과 수신 핸들러가 겹치지 않게 하는 것이다.

## 대안과 비교

| 방식 | 장점 | 단점 |
|---|---|---|
| **세션별 strand (현재)** | 소켓 만들 때 한 줄로 끝. 핸들러는 자동으로 직렬화됨 | 유휴 상태에서 전송을 시작할 때 `post` 1회 |
| 소켓을 쓰는 곳마다 뮤텍스 | 개념이 익숙함 | 모든 사용처에 락 코드가 필요하고, 한 곳이라도 빠뜨리면 버그 |
| I/O 스레드를 1개로 고정 | strand가 필요 없음 | 로직 스레드의 `Send()`는 여전히 `post`가 필요하고, I/O 처리량이 스레드 하나에 묶임 |
| 세션을 특정 I/O 스레드에 고정 (스레드당 `io_context`) | strand 없이도 겹치지 않음 | 엔진 구조를 바꿔야 하고 세션 분배 로직이 추가됨 |

## 관련 코드 위치

- `02_ServerEngine/Source/Boost/BoostSession.cpp:5` — 소켓에 세션 strand 연결
- `02_ServerEngine/Source/Boost/BoostSession.cpp:27-64` — `Send()`: 큐에 넣고 strand로 `post`
- `02_ServerEngine/Source/Boost/BoostSession.cpp:66-90` — `FlushSendQueue()`: strand 위에서 `async_write` 시작
- `02_ServerEngine/Source/Boost/BoostSession.cpp:92-109` — `CompleteSend()`: strand 위에서 실행되는 완료 핸들러
- `02_ServerEngine/Include/Boost/BoostSession.h:40-47` — 세션 멤버 변수
- `02_ServerEngine/Source/Boost/BoostNetEngine.cpp:25-30` — `io_context`를 여러 I/O 스레드가 실행

관련 문서: [BoostSession 송신 큐 / SendBuffer](../ClassExplanation/02_ServerEngine/Boost/BoostSession_SendQueue.md)