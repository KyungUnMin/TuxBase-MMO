# SO_LINGER 소켓 옵션

## 개요

`SO_LINGER`는 소켓을 `close()` 할 때 **남은 데이터를 어떻게 처리할지** 결정하는 옵션이다.
이 옵션에 따라 TCP 연결 종료 방식이 완전히 달라진다.

## TCP 연결 종료 방식: FIN vs RST

### 정상 종료 (Graceful Close) — FIN 패킷

```
A → B : FIN     (나 더 보낼 거 없어)
B → A : ACK     (알겠어)
B → A : FIN     (나도 없어)
A → B : ACK     (알겠어, 끝)
```

- **4-way handshake**로 양쪽 합의하에 종료
- 종료 후 소켓은 **TIME_WAIT** 상태로 60~120초 동안 잔류
- 송신 버퍼에 남은 데이터를 **다 보낸 뒤** 종료

### 강제 종료 (Abortive Close) — RST 패킷

```
A → B : RST     (즉시 끊는다)
       (끝. 1패킷으로 완료)
```

- 상대방 응답을 기다리지 않고 **즉시 종료**
- **TIME_WAIT가 발생하지 않음** → 소켓 리소스 즉시 회수
- 송신 버퍼에 남은 데이터는 **전부 폐기**
- 4-way handshake를 **시도조차 하지 않음**

## linger 설정별 동작 비교

| 설정 | close() 시 동작 | 종료 방식 | TIME_WAIT | 남은 데이터 |
|------|-----------------|----------|-----------|------------|
| 미설정 (OS 기본) | FIN 전송 | 4-way handshake | **발생** (60~120초) | 전송 시도 |
| `linger(true, 0)` | RST 전송 | 즉시 종료 | **없음** | 폐기 |
| `linger(true, N)` | FIN 전송, N초 대기 | 4-way 시도, 초과 시 RST | 초과 시 없음 | N초간 전송 시도 |

## 게임 서버에서의 선택

게임 서버에서는 **`linger(true, 0)`을 사용하는 경우가 많다.**

### 이유

1. **TIME_WAIT 회피**: 동접 수천~수만 명인 서버에서 TIME_WAIT 소켓이 쌓이면 포트/파일 디스크립터 고갈로 이어질 수 있음
2. **리소스 빠른 회수**: 세션이 끊기면 어차피 재접속해야 하므로, 소켓을 빠르게 회수해서 다른 클라이언트에게 할당하는 것이 더 중요
3. **단순함**: graceful shutdown 대기 로직이 불필요

### 주의사항

- `close()` 시 송신 버퍼에 남은 데이터가 유실될 수 있음
- 연결 종료 전 반드시 보내야 하는 마지막 패킷이 있다면, 전송 완료를 확인한 뒤 `close()`를 호출해야 함

## Boost.Asio에서의 설정

```cpp
// linger(true, 0) — RST로 즉시 종료
boost::asio::socket_base::linger option(true, 0);
socket.set_option(option);

// linger(true, 5) — FIN 시도, 최대 5초 대기
boost::asio::socket_base::linger option(true, 5);
socket.set_option(option);
```
