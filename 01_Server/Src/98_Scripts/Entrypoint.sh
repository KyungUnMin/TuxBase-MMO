#!/bin/bash

TARGET=$1
if [ "$TARGET" != "server" ] && [ "$TARGET" != "client" ]; then
    echo "Error: 타겟을 지정해주세요 (server 또는 client)"
    exit 1
fi

LOG_DIR="/root/src/build/${TARGET}"
mkdir -p "$LOG_DIR"
LOG_FILE="${LOG_DIR}/FullBuild.log"

echo "=== Starting CMake FullBuild for $TARGET ===" > "$LOG_FILE"

cd /root/src

echo "1. Configuring CMake..." >> "$LOG_FILE"
cmake -S . -B "build/${TARGET}/Linux" -G Ninja -DCMAKE_BUILD_TYPE=Debug >> "$LOG_FILE" 2>&1

echo "2. Building..." >> "$LOG_FILE"
cmake --build "build/${TARGET}/Linux" -j >> "$LOG_FILE" 2>&1

echo "=== FullBuild Finished ===" >> "$LOG_FILE"

exec sleep infinity
