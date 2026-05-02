Clear-Host
Write-Host "===== Running Containers =====" -ForegroundColor Cyan
docker ps

Write-Host "`n===== Stopping and Removing Dev Containers =====" -ForegroundColor Yellow
docker compose -p 01_server_devcontainer -f compose.yml down
Write-Host "Dev containers stopped." -ForegroundColor Green

Write-Host "`n===== Running Containers (after stop) =====" -ForegroundColor Cyan
docker ps
