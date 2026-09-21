#!/bin/bash

# 컨테이너 일괄 실행 스크립트

# 스크립트가 있는 디렉터리
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# compose.yml, .env가 있는 디렉터리 (scripts/의 상위)
COMPOSE_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$COMPOSE_DIR" || exit 1

# IMAGE_NAME 로드 (rebuild_docker_image.sh가 만드는 이미지 이름)
source "$COMPOSE_DIR/.env" || exit 1

clear
if ! docker image inspect "$IMAGE_NAME" > /dev/null 2>&1; then
    echo -e "\033[31mDocker image '$IMAGE_NAME' not found. Run scripts/rebuild_docker_image.sh first.\033[0m"
    exit 1
fi

echo -e "\033[36mStarting all Dev Containers\033[0m"

if docker compose -p 01_server_devcontainer -f compose.yml up -d; then
    echo -e "\033[32mContainers started successfully!\033[0m"
else
    echo -e "\033[31mFailed to start containers. Please check the error messages above.\033[0m"
    exit 1
fi
