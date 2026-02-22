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
        pkg-config \
    && dnf clean all \
    && rm -rf /var/cache/dnf

# vcpkg 설치
WORKDIR /opt 
RUN git clone https://github.com/microsoft/vcpkg.git && \
    ./vcpkg/bootstrap-vcpkg.sh

# PATH 등록
ENV VCPKG_ROOT=/opt/vcpkg
ENV PATH="${VCPKG_ROOT}:${PATH}"

# 소스 마운트 디렉터리를 기본 작업 디렉터리로 설정
WORKDIR /root/src

# 컨테이너 유지 (개발용)
ENTRYPOINT ["/bin/bash", "-c", "sleep infinity"]