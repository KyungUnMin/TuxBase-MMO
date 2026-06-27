# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 언어

모든 의사소통은 **한국어**로 진행합니다.

## 빌드 및 개발 워크플로우

소스는 Windows에서 작성하고, **Docker 컨테이너(Fedora Linux)** 안에서 빌드/실행합니다.

### Docker 컨테이너 시작

```bash
# 01_Server/.devcontainer/ 에서 실행
docker compose up -d
docker exec -it tuxbase-mainserver zsh
```

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

### 프로토버프 컴파일

`.proto` 파일은 `03_Share/Protocol/` 에 위치합니다. 수정 후 protoc로 재생성:

```bash
protoc --cpp_out=<출력경로> 03_Share/Protocol/Example.proto
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
- **스타일**: Allman 중괄호, `#pragma once`, 헤더 `.hpp`/소스 `.cpp`
- **네이밍**: 클래스 `PascalCase`, 인터페이스 `I` 접두어, 멤버 `m_camelCase`, 상수 `kPascalCase`
- **접근 제어**: 멤버 변수는 `private` 고정, `protected` 멤버 변수 지양
- **파일 헤더 주석 금지**, **파일 끝 공백 금지**

## 기술 스택

- **C++20**, Boost.Asio, Protobuf, fmt, GTest
- **패키지 관리**: vcpkg (매니페스트 모드, `vcpkg.json`)
- **빌드**: CMake 3.20+, Ninja
- **런타임**: Docker + Fedora Linux 컨테이너
