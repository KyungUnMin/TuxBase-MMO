@echo off
echo ========================================
echo  TuxBase-MMO Docker Build Environment
echo ========================================
echo.

echo [1/3] Docker Compose 빌드 및 시작 ...
docker compose up -d --build

echo [2/3] 컨테이너 상태 확인 ...
docker compose ps

echo.
echo [3/3] 컨테이너 접속 ...
echo   빌드 명령어:
echo     cmake -B build -G Ninja
echo     cmake --build build
echo.
docker exec -it 01_docker-mytestserver-1 bash
