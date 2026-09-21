#!/bin/bash

# 스크립트가 있는 디렉터리
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# compose.yml, .env가 있는 디렉터리 (scripts/의 상위)
COMPOSE_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$COMPOSE_DIR" || exit 1

clear
echo -e "\033[36m===== Running Containers =====\033[0m"
docker ps

echo -e "\n\033[33m===== Stopping and Removing Dev Containers =====\033[0m"
docker compose -p 01_server_devcontainer -f compose.yml down
echo -e "\033[32mDev containers stopped.\033[0m"

echo -e "\n\033[36m===== Running Containers (after stop) =====\033[0m"
docker ps
