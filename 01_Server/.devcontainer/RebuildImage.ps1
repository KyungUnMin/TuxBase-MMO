$images = docker images -q
if ($images) 
{ 
    docker image rm -f $images 
    Write-Host "Complete deletion of all images" -ForegroundColor Green
}
else 
{
     Write-Host "There are no images to delete." -ForegroundColor Yellow
}

docker build -t tuxbase-mmo-dev .