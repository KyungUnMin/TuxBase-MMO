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

# Claude Code CLI 설치 (인증/설정은 compose.yml의 bind mount로 유지)
RUN curl -fsSL https://claude.ai/install.sh | bash
ENV PATH="/root/.local/bin:${PATH}"

# 컨테이너 내 사용자와 저장소 소유자가 달라도 Git 저장소를 신뢰하도록 설정
RUN git config --global --add safe.directory /root/tuxbase_mmo

# vcpkg 설치
WORKDIR /opt 
RUN git clone https://github.com/microsoft/vcpkg.git && \
    ./vcpkg/bootstrap-vcpkg.sh

# PATH 등록
ENV VCPKG_ROOT=/opt/vcpkg
ENV PATH="${VCPKG_ROOT}:${PATH}"

# 소스 마운트 디렉터리를 기본 작업 디렉터리로 설정
WORKDIR /root/tuxbase_mmo
