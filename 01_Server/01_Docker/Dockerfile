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
    && dnf clean all \
    && rm -rf /var/cache/dnf

# vcpkg 설치
WORKDIR /opt 
RUN git clone https://github.com/microsoft/vcpkg.git && \
    ./vcpkg/bootstrap-vcpkg.sh

# PATH 등록
ENV VCPKG_ROOT=/opt/vcpkg
ENV PATH="${VCPKG_ROOT}:${PATH}"

ENTRYPOINT ["/bin/bash", "-c", "sleep 500"]