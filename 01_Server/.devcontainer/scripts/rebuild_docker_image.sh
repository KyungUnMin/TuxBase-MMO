#!/bin/bash

# rebuild_docker_image.sh가 있는 디렉터리
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Dockerfile, compose.yml, .env가 있는 디렉터리 (scripts/의 상위)
BUILD_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

# IMAGE_NAME 로드
source "$BUILD_DIR/.env" || exit 1

clear
echo -e "\033[33mPrev Docker Image List\033[0m"
docker image ls

if docker image inspect "$IMAGE_NAME" > /dev/null 2>&1; then
    docker image rm -f "$IMAGE_NAME"
    echo -e "\033[32mComplete deletion of $IMAGE_NAME\033[0m"
else
    echo -e "\033[33mThere is no $IMAGE_NAME image to delete.\033[0m"
fi

docker build -t "$IMAGE_NAME" "$BUILD_DIR"

if [ $? -eq 0 ]; then
    echo -e "\033[32mDocker image build complete.\033[0m"
else
    echo -e "\033[31mDocker image build failed.\033[0m"
    exit 1
fi

echo -e "\033[33mNow Docker Image List\033[0m"
docker image ls