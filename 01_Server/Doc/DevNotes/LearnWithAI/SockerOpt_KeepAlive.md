# SO_KEEPALIVE 소켓 옵션

## 개요

`SO_KEEPALIVE`를 켜면 OS 커널이 주기적으로 **keepalive probe** 패킷을 보내서 TCP 연결이 살아있는지 확인한다.
상대방이 응답하지 않으면 연결이 끊어진 것으로 판단하고 소켓을 정리한다.

## OS keepalive 기본 설정값

| 파라미터 | 기본값 | 설명 |
|---------|--------|------|
| `tcp_keepalive_time` | **2시간** | 첫 probe까지 대기 시간 |
| `tcp_keepalive_intvl` | **75초** | probe 간격 |
| `tcp_keepalive_probes` | **9회** | 최대 재시도 횟수 |

기본값 그대로라면 클라이언트가 비정상 종료(랜선 뽑힘, 정전 등)해도 **최소 2시간 + α** 후에야 감지된다.
`setsockopt`으로 `TCP_KEEPIDLE`, `TCP_KEEPINTVL`, `TCP_KEEPCNT`를 조절할 수 있지만, 플랫폼마다 지원이 다르다.

## OS keepalive vs 애플리케이션 heartbeat

| 비교 항목 | OS keepalive | 앱 레벨 heartbeat |
|-----------|-------------|-------------------|
| 감지 속도 | 기본 2시간+ (튜닝 필요) | **자유롭게 설정** (5~30초) |
| 플랫폼 독립성 | OS마다 파라미터 다름 | **완전 통제 가능** |
| 감지 범위 | TCP 연결 생존 여부만 | **앱 수준 생존 확인** (프리징 감지 가능) |
| 구현 복잡도 | 옵션 한 줄 | 타이머 + 패킷 구현 필요 |

### 핵심 차이

**"TCP 연결이 살아있다 ≠ 클라이언트가 정상 동작 중이다"**

- 클라이언트 프로세스가 무한루프에 빠지거나 프리징되어도 OS TCP 스택은 keepalive probe에 **정상 응답**한다.
- 앱 레벨 heartbeat는 클라이언트가 실제로 응답 패킷을 **직접 만들어서 보내야** 하므로 더 정확하다.

### 대체 가능 여부

**heartbeat는 SO_KEEPALIVE를 완전히 대체할 수 있다** (상위호환).

- TCP 연결이 끊어진 경우 → heartbeat 패킷 전송 자체가 실패 → 감지됨
- TCP는 살아있지만 클라이언트가 프리징 → heartbeat 응답이 안 옴 → 감지됨
- 클라이언트가 연결만 유지하고 아무것도 안 함 → 감지됨

SO_KEEPALIVE로 잡히는 케이스는 heartbeat로 100% 잡힌다.

## Connect만 하고 데이터를 안 보내는 공격 (DDoS 시나리오)

### 시나리오 A: TCP SYN Flood (3-way handshake 미완료)

```
공격자 → 서버 : SYN
서버 → 공격자 : SYN-ACK
공격자 :        (ACK를 안 보냄)
```

- 소켓이 `accept()`되지 않으므로 애플리케이션 코드까지 도달하지 않음
- **SO_KEEPALIVE, heartbeat 모두 무관** — OS/방화벽 레벨(SYN cookies, iptables 등)에서 대응

### 시나리오 B: Connect 완료 후 아무 데이터도 안 보냄

```
공격자 → 서버 : SYN → SYN-ACK → ACK  (3-way 완료, accept 성공)
공격자 :                               (이후 아무것도 안 보냄)
```

- 서버의 `accept()`가 성공하고 세션 풀 슬롯을 차지
- 반복되면 세션 풀 고갈
- **SO_KEEPALIVE는 이 공격에 무력함** — 공격자의 TCP 연결은 "살아있는" 상태이므로 keepalive probe에 정상 응답

### 대응 방법: 인증 타임아웃 + 하트비트 타임아웃

| 단계 | 타이머 | 용도 |
|------|--------|------|
| **인증 타임아웃** | 접속 후 5~10초 | Connect 후 인증 패킷을 보내지 않으면 강제 종료 |
| **하트비트 타임아웃** | 인증 후 30~60초 | 인증된 세션이 heartbeat를 안 보내면 종료 |

```
[Accept 완료] → 5초 타이머 시작
                  ├─ 5초 내 인증 패킷 수신 → 인증 성공 → heartbeat 모드 전환
                  └─ 5초 초과 → 강제 disconnect (좀비 세션 / 공격 차단)
```

인증 타임아웃이 heartbeat보다 **더 중요**하다.
heartbeat는 "인증된 정상 클라이언트"를 감시하는 것이고, 인증 타임아웃은 "정체불명의 연결"을 빠르게 쳐내는 것이다.

## 게임 서버에서의 권장 전략

1. **heartbeat 미구현 단계**: `SO_KEEPALIVE`를 켜두고 최소한의 안전망으로 활용
2. **heartbeat 구현 후**: `SO_KEEPALIVE`는 꺼도 무방 (둘 다 켜놔도 문제없음)
3. **반드시 구현**: 인증 타임아웃 (connect 후 N초 내 인증 없으면 강제 종료)

## Boost.Asio에서의 설정

```cpp
// SO_KEEPALIVE 활성화
socket.set_option(boost::asio::socket_base::keep_alive(true));
```
