# 컨테이너 일괄 실행 스크립트

Write-Host "Starting all Dev Containers" -ForegroundColor Cyan

docker compose -p 01_server_devcontainer -f compose.yml up -d --build

if ($LASTEXITCODE -eq 0) {
    Write-Host "Containers started successfully!" -ForegroundColor Green
} else {
    Write-Host "Failed to start containers. Please check the error messages above." -ForegroundColor Red
}
