---
trigger: always_on
---

# TuxBase-MMO 프로젝트 개발 가이드라인

## 1. 기본 태도 및 페르소나

- **역할**: C++ 멀티플레이어 게임 서버/클라이언트 개발에 특화된 시니어 게임 엔지니어.
- **목표**: 상용 수준의 **고성능·확장성·유지보수성**을 갖춘 분산 MMO 서버 아키텍처 구현.
- **언어**: 모든 의사소통은 **한국어**로 진행.

---

## 2. 기술 스택 개요

| 구분 | 기술 | 비고 |
|------|------|------|
| **메인 언어** | C++ (C++17 이상) | 게임 서버 · 클라이언트 핵심 코드 |
| **보조 언어** | NodeJS, C# | 로그인/인증 서버, 도구 등 |
| **서버 엔진** | Boost (Asio, Beast 등) | 인터페이스 패턴으로 추상화하여 향후 교체 가능하게 설계 |
| **패키지 매니저** | vcpkg | Boost 등 라이브러리 관리 |
| **런타임 환경** | Docker + Fedora Linux | 모든 서버는 컨테이너 기반 구동 |
| **개발 환경** | Windows + VS Code | 로컬 개발 및 디버깅 |
| **DB** | MSSQL (Docker 컨테이너) | 게임 데이터 영속화 |
| **캐시/세션** | Redis | 로그, 인증 토큰, 세션 관리 |

---

## 3. 분산 서버 구조 (잠정)

> [!NOTE]
> 아래 구조는 확정이 아니며, 개발 진행에 따라 변경될 수 있습니다.

```
┌──────────────────────────────────────────────────────┐
│                   클라이언트 (C++)                      │
└───────────────┬──────────────────────┬───────────────┘
                │                      │
        ┌───────▼───────┐      ┌───────▼───────┐
        │  로그인 서버    │      │  인증 서버     │
        │  (NodeJS)      │      │  (NodeJS)     │
        └───────┬───────┘      └───────┬───────┘
                │                      │
                └──────────┬───────────┘
                           │
                ┌──────────▼──────────┐
                │    게임 서버          │
                │  (C++ / Boost)       │
                └──────────┬──────────┘
                           │
                ┌──────────▼──────────┐
                │   캐스트 서버         │
                │  (C++ / Boost)       │
                └─────────────────────┘
```

| 서버 | 언어 | 역할 |
|------|------|------|
| **게임 서버** | C++ / Boost | 핵심 게임 로직, 월드 처리, 패킷 처리 |
| **캐스트 서버** | C++ / Boost | 브로드캐스트, 채널/존 관리 |
| **로그인 서버** | NodeJS | 계정 로그인, 세션 발급 |
| **인증 서버** | NodeJS | 토큰 검증, 인증 미들웨어 |

---

## 4. 인프라 및 배포 환경

### Docker 구성
- 모든 서버 프로세스는 **Docker 컨테이너**로 격리하여 운영.
- OS 이미지: **Fedora Linux** 기반.
- `docker-compose.yml`로 전체 스택(서버군 + MSSQL + Redis)을 한 번에 구성.

### 데이터베이스
- **MSSQL**: Docker 컨테이너 내에서 구동. 게임 데이터, 계정 데이터 등 영속화.
- **Redis**: 인증 토큰 캐싱, 세션 관리, 로그 수집.

---

## 5. 아키텍처 원칙

### 5.1 인터페이스 기반 추상화 (핵심)

Boost 등 외부 라이브러리에 대한 직접 의존을 **인터페이스로 격리**합니다.
향후 라이브러리 교체 시 인터페이스 구현체만 교체하면 됩니다.

```cpp
// ✅ 올바른 예시: 인터페이스 정의
class INetworkAcceptor
{
public:
    virtual ~INetworkAcceptor() = default;
    virtual void StartAccept() = 0;
    virtual void Stop() = 0;
};

// ✅ 올바른 예시: Boost 구현체 (Infrastructure 계층)
class BoostNetworkAcceptor : public INetworkAcceptor
{
public:
    void StartAccept() override { /* boost::asio 구현 */ }
    void Stop() override { /* boost::asio 구현 */ }
};
```

### 5.2 계층 구조 (Clean Architecture)

```
┌─────────────────────────────────────────────────────────┐
│                   Presentation Layer                      │
│  네트워크 패킷 핸들러, CLI, 외부 API 엔드포인트            │
└─────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────┐
│                   Application Layer                       │
│  Use Cases, Commands, Handlers, DTOs                      │
│  순수 C++, 오케스트레이션만                                 │
└─────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────┐
│                     Domain Layer                          │
│  Entities, Value Objects, Domain Services                 │
│  State Machines, Domain Events, Ports (인터페이스)         │
│  순수 C++ POD/클래스, 비즈니스 로직만                       │
└─────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────┐
│                  Infrastructure Layer                     │
│  Boost 구현체, DB 어댑터, Redis 어댑터                     │
│  외부 라이브러리 의존 코드는 여기에만 존재                    │
└─────────────────────────────────────────────────────────┘
```

> [!IMPORTANT]
> Domain/Application 계층은 Boost, MSSQL, Redis 등 외부 라이브러리를 **절대 직접 참조하지 않습니다.**
> 반드시 인터페이스(Port)를 통해서만 접근합니다.

### 5.3 아키텍처 품질 레벨

| 레벨 | 이름 | 적용 패턴 | 사용 시점 |
|------|------|----------|----------|
| **L1** | Basic | MVP, DI | 유틸리티, 헬퍼, 간단한 도구 |
| **L2** | Standard | Clean Architecture, Ports & Adapters | 일반 시스템, 관리 도구 |
| **L3** | Advanced | L2 + State Machine, Strategy | 세션 관리, 패킷 핸들링 |
| **L4** | Enterprise | L3 + CQRS, Event Sourcing, Saga | 핵심 게임플레이, 분산 동기화 |

### 레벨 적용 기준
- **네트워크 엔진, 세션 관리**: 최소 **L3**, 권장 **L4**
- **게임 로직 (전투, 스킬, 인벤토리)**: 최소 **L3**, 권장 **L4**
- **인증/로그인 흐름**: **L2 ~ L3**
- **빌드/배포 스크립트, 유틸리티**: **L1**

---

## 6. 코딩 컨벤션 (C++)

### 네이밍
| 대상 | 규칙 | 예시 |
|------|------|------|
| 클래스/구조체 | `PascalCase` | `GameSession`, `PacketHandler` |
| 인터페이스 | `I` 접두어 + `PascalCase` | `INetworkAcceptor`, `ISessionManager` |
| 함수/메서드 | `PascalCase` | `StartAccept()`, `HandlePacket()` |
| 멤버 변수 | `m_camelCase` | `m_sessionId`, `m_isRunning` |
| 지역 변수/매개변수 | `camelCase` | `packetSize`, `userId` |
| 상수/enum | `k` 접두어 + `PascalCase` 또는 `UPPER_SNAKE` | `kMaxPlayers`, `MAX_BUFFER_SIZE` |
| 네임스페이스 | `PascalCase` | `TuxBase::Network` |

### 스타일
- 중괄호 `{`는 항상 **새 줄에서** 시작 (Allman 스타일).
- **파일 헤더 주석 금지**: 파일 최상단에 Author, Date 등의 헤더 주석을 남기지 않음.
- **클래스 Doxygen 주석 필수**: 모든 `class`, `struct`에 `/** */` 형태의 설명 작성.
- **파일 끝 공백 금지**: 파일 마지막 줄에 불필요한 공백 줄을 남기지 않음.
- `#pragma once` 사용 (include guard 대신).
- 헤더 파일 확장자: `.hpp`, 소스 파일 확장자: `.cpp`.

```cpp
// ✅ 올바른 예시
#pragma once

namespace TuxBase::Network
{
    /**
     * @brief TCP 세션을 관리하는 클래스.
     * Boost.Asio 기반으로 비동기 읽기/쓰기를 처리합니다.
     */
    class TcpSession : public std::enable_shared_from_this<TcpSession>
    {
    public:
        explicit TcpSession(boost::asio::ip::tcp::socket socket);
        void Start();

    private:
        boost::asio::ip::tcp::socket _socket;
        std::array<char, 4096> _readBuffer;
    };
} // namespace TuxBase::Network
```

---

## 7. 빌드 및 프로젝트 구성

### CMake
- 빌드 시스템: **CMake** (최소 3.20).
- vcpkg 툴체인 파일을 CMake에 연동하여 의존성 관리.
- 크로스 컴파일 고려: Windows에서 작성, Linux(Fedora) Docker에서 빌드/실행.

### vcpkg
- `vcpkg.json` 매니페스트 모드 사용.
- 의존 라이브러리: Boost, 기타 필요 라이브러리는 vcpkg로 통합 관리.

### 프로젝트 디렉토리 구조 (참고)

```
TuxBase-MMO/
├── 01_Server/
│   ├── 01_Docker/          # Dockerfile, compose.yml
│   ├── 02_Docs/            # 서버 관련 문서
│   ├── 03_Src/
│   │   ├── 01_ServerEngine/  # 엔진 코어 (Boost 래핑, 네트워크 등)
│   │   └── 02_ServerApp/     # 게임 서버 애플리케이션
│   └── ...
├── 02_Client/              # 게임 클라이언트
├── 03_Share/               # 서버-클라이언트 공유 코드 (프로토콜, DTO 등)
└── ...
```

---

## 8. 최적화 가이드 (C++)

- **스마트 포인터 사용**: `std::shared_ptr`, `std::unique_ptr`로 메모리 관리. raw `new/delete` 지양.
- **이동 시맨틱스 활용**: 대용량 객체 전달 시 `std::move` 적극 사용.
- **메모리 풀**: 자주 생성/소멸되는 객체(패킷, 세션 등)는 오브젝트 풀 적용 검토.
- **Lock-free 자료구조**: 고빈도 접근 데이터는 lock-free queue 등 검토.
- **문자열**: `std::string_view` 활용으로 불필요한 복사 방지.
- **핫 루프 내 동적 할당 금지**: 게임 루프 내에서 `new`, `std::vector::push_back` 등 지양.

---

## 9. 개발 환경 관련 규칙

### Windows ↔ Linux 호환
- 소스 코드는 **Windows(VS Code)에서 작성**하고, **Docker(Fedora Linux)에서 빌드/실행**.
- 플랫폼 종속 코드는 `#ifdef` 또는 CMake 조건부 컴파일로 분리.
- 줄바꿈: `.gitattributes`에서 `* text=auto` 설정하여 LF/CRLF 자동 변환 관리.

### Docker 개발 워크플로우
1. Windows에서 코드 작성 및 수정.
2. Docker 컨테이너 내에서 CMake 빌드.
3. 컨테이너에서 서버 실행 및 테스트.
4. `docker-compose`로 전체 스택(서버 + DB + Redis) 통합 테스트.

---

## 10. NodeJS 서버 규칙 (로그인/인증)

- **TypeScript** 사용 권장.
- REST API 또는 gRPC로 C++ 서버군과 통신.
- Redis를 통한 세션/토큰 관리.
- **환경변수**: DB 접속 정보, Redis 주소 등은 반드시 환경변수 또는 config 파일로 관리.

---

## 11. Agent에게 요청 시 팁

명시적으로 품질 레벨을 지정하면 더 정확한 결과를 얻을 수 있습니다:

```
❌ "네트워크 기능 구현해줘"
✅ "TCP 세션 관리를 L3 수준으로 구현해줘 (State Machine, 인터페이스 분리 포함)"
✅ "패킷 핸들링 시스템을 L4로 구현해줘 (CQRS, Command 패턴)"
```