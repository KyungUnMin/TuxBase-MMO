# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 언어

모든 의사소통은 **한국어**로 진행합니다.

## 프로젝트 성격

이 프로젝트는 실제 서비스가 아닌 **게임 서버 개발자 포트폴리오 제출용**입니다.
대규모 트래픽 대비, 분산/이중화, 상용 모니터링·배포 파이프라인 등 실서비스 운영 관점의 과잉 엔지니어링은 지양하고, 포트폴리오로 보여줄 핵심 기능/아키텍처에 집중해서 제안합니다.

사용자(개발자 본인)가 이 스코프에 비해 과한 설계나 기능을 요청하는 경우에도, 그대로 따르지 말고 "포트폴리오 가치 대비 들어가는 공수가 적절한지" 먼저 짚어주고 트레이드오프를 제시합니다. 사용자가 이유를 듣고도 진행을 원하면 그때 구현합니다.

## 기술 제안 방식

기술적인 제안을 할 때는 항상 대안들의 **트레이드오프를 비교해서 나열**합니다.

## 빌드 및 개발 워크플로우

**WSL** 안에서 Docker를 띄운 후, **Docker 컨테이너(Fedora Linux)** 내부에서 빌드/실행합니다.

### Docker 컨테이너 시작

```bash
# 01_Server/.devcontainer/ 에서 실행
docker compose -p 01_server_devcontainer -f compose.yml up -d --build
```

컨테이너 종료:

```bash
docker compose -p 01_server_devcontainer -f compose.yml down
```

VSCode의 **Reopen in Container** 기능으로 컨테이너에 연결해서 작업합니다.

### 컨테이너 내 빌드

```bash
# /root/src 에서 실행 (소스가 볼륨 마운트된 위치)
cmake -S . -B build/server/Linux -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/server/Linux
```

빌드 결과물은 `build/server/Linux/` 하위에 생성됩니다.

### 테스트 실행

```bash
# 전체 테스트
./build/server/Linux/99_Test/Tests

# 특정 테스트 필터링
./build/server/Linux/99_Test/Tests --gtest_filter=<TestSuiteName>.*
```

## 프로젝트 구조 (서버)

```
01_Server/Src/
├── 01_ServerBase/     # 플랫폼 독립적 기반 유틸리티 (정적 라이브러리)
├── 02_ServerEngine/   # Boost.Asio 기반 네트워크 엔진 (정적 라이브러리)
├── 03_ServerApp/      # 게임 서버 애플리케이션 (실행 파일)
├── 04_DummyClient/    # 테스트용 더미 클라이언트 (실행 파일)
└── 99_Test/           # GTest 기반 단위 테스트 (실행 파일)

03_Share/Protocol/     # .proto 파일 (서버-클라이언트 공유)
```

의존 관계: `ServerApp` → `ServerEngine` → `ServerBase`

## 아키텍처 핵심

### 계층 구조 (Clean Architecture)

- **Infrastructure**: Boost 구현체 (`02_ServerEngine/Include/Boost/`)
- **Interface(Port)**: `INetEngine`, `ISession` (`EngineInterface/`)
- **Domain/Application**: `03_ServerApp/` — Boost 직접 참조 금지, 인터페이스만 사용

외부 라이브러리 의존 코드는 Infrastructure 계층(`Boost/` 디렉토리)에만 존재합니다.

### 네트워크 엔진 흐름

1. `BoostNetEngine` (INetEngine 구현체): io_context + 스레드풀 + 세션 풀 관리
2. `BoostSession` (ISession 구현체): 소켓 per 세션, `RingBuffer` 기반 recv/send 버퍼
3. `PacketSerializer`: `RingBuffer` ↔ Protobuf 메시지 직렬화/역직렬화
4. `PacketDispatcherBase`: 수신 패킷을 `LockQueue`에 적재 후 `Dispatch()` 가상함수로 처리

### 패킷 포맷

`[PacketHeader(4 bytes)] + [Protobuf Body]`  
헤더: `m_size(2B) + m_id(2B)`, `#pragma pack(1)` 적용.  
패킷 ID는 `03_Share/Protocol/Example.proto`의 `PacketId` enum과 매핑.

## 코딩 컨벤션 (C++)

- **표준**: C++20, `auto` 사용 금지 (타입 명시)
- **스타일**: Allman 중괄호, `#pragma once`, 헤더 `.h`/소스 `.cpp`
- **네이밍**: 클래스 `PascalCase`, 인터페이스 `I` 접두어, 멤버 `m_camelCase`, 상수 `kPascalCase`
- **접근 제어**: 멤버 변수는 `private` 고정, `protected` 멤버 변수 지양
- **파일 헤더 주석 금지**, **파일 끝 공백 금지**
- **표준 라이브러리 우선 검토**: 외부 라이브러리(fmt 등)로 쓰던 기능이 C++ 표준(std::format/std::print 등)에 동등하게 들어와 있다면, 빌드 환경(vcpkg, 컨테이너의 GCC 버전)이 지원하는지 먼저 확인한 뒤 표준 라이브러리 사용을 우선 검토합니다. 지원 여부가 불확실하면 바로 바꾸지 말고 먼저 확인 결과를 공유합니다.

## 문서화 규칙

- 핵심 클래스의 설계나 공개 API가 바뀌면, `01_Server/Doc/DevNotes/ClassExplanation/` 아래 대응하는 문서도 같이 갱신합니다 (문서와 코드가 어긋난 채로 방치하지 않습니다).
- README나 DevNotes 문서·주석에 "빠르다", "효율적이다" 같은 성능 관련 주장을 적을 때는 실측 벤치마크 수치를 함께 남깁니다. 근거 없이 성능을 주장하지 않습니다.

## 기술 스택

- **C++20**, Boost.Asio, Protobuf, fmt, GTest
- **패키지 관리**: vcpkg (매니페스트 모드, `vcpkg.json`)
- **빌드**: CMake 3.20+, Ninja
- **런타임**: Docker + Fedora Linux 컨테이너

### 향후 계획 (미확정)

- 로그인 서버, 빌링 서버, DB 쿼리 서버는 이후 **Node.js**로 만들 예정입니다. 아직 착수 전이며, 전체 서버 아키텍처(프로세스 구성 등)는 확정되지 않았습니다.
