@echo off

set BUILD_TYPE=Debug
if not "%1"=="" set BUILD_TYPE=%1

cls

echo Building LOVE Potion for Wii U using Docker (%BUILD_TYPE% build)...

echo Starting Docker...
docker desktop start

REM Clean up old images first
echo Cleaning up old Docker images...
docker rmi lovepotion-wiiu 2>nul || echo No old image to remove

REM Build Docker image
echo Building Docker image...
docker build --build-arg BUILD_TYPE=%BUILD_TYPE% -f Dockerfile.wiiu -t lovepotion-wiiu .

if %ERRORLEVEL% neq 0 (
    echo Docker build failed! 
    pause
    exit /b 1
)



REM Create build directory if it doesn't exist
if not exist "build" mkdir build

REM Run container and copy files
echo Extracting built files...
echo Running: docker run --rm -v "%CD%\build:/host/build" lovepotion-wiiu sh -c 
docker run --rm -v "%CD%\build:/host/build" lovepotion-wiiu sh -c 

echo Checking build directory...
dir /b build

cd
.\extract-build.bat
pause