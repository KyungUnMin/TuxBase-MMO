Clear-Host
Write-Host "===== Running Containers =====" -ForegroundColor Cyan
docker ps

Write-Host "`n===== Stopping All Containers =====" -ForegroundColor Yellow
$ids = docker ps -q
if ($ids) 
{
    docker stop $ids
    Write-Host "All containers stopped." -ForegroundColor Green
} 
else 
{
    Write-Host "No running containers." -ForegroundColor Gray
    return
}

Write-Host "`n===== Running Containers (after stop) =====" -ForegroundColor Cyan
docker ps

Write-Host "`n===== All Containers =====" -ForegroundColor Cyan
docker ps -a
