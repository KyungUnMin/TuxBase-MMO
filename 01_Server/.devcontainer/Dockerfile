FROM fedora:43

# 필수 도구 설치
RUN dnf update -y \
    && dnf install -y \
    sudo \
    git \
    zsh \
    which \
    shadow-utils \
    curl \
    zip \
    unzip \
    tar \
    gcc \
    gcc-c++ \
    make \
    cmake \
    ninja-build \
    gdb \
    clang \
    clang-tools-extra \
    lldb \
    llvm \
    libubsan \
    libunwind-devel \
    pkg-config \
    && dnf clean all \
    && rm -rf /var/cache/dnf

# Oh My Zsh 설치 및 기본 셸 설정
RUN sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)" "" --unattended \
    && chsh -s $(which zsh)

# Git CRLF(윈도우 줄바꿈)랑 LF(리눅스 줄바꿈) 자동 변환
RUN git config --global core.autocrlf true

# vcpkg 설치
WORKDIR /opt 
RUN git clone https://github.com/microsoft/vcpkg.git && \
    ./vcpkg/bootstrap-vcpkg.sh

# PATH 등록
ENV VCPKG_ROOT=/opt/vcpkg
ENV PATH="${VCPKG_ROOT}:${PATH}"

# 소스 마운트 디렉터리를 기본 작업 디렉터리로 설정
WORKDIR /root/src
