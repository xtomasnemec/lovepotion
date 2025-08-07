@echo off
echo Extracting built files from Docker container...

REM Create build directory if it doesn't exist
if not exist "build" mkdir build

REM Extract files from Docker container
docker run --rm -v "%cd%\build:/host/build" lovepotion-wiiu sh -c "echo '=== COPYING FILES ===' && cp -v /output/* /host/build/ 2>/dev/null || echo 'No files in /output' && cp -v /project/build/*.wuhb /host/build/ 2>/dev/null || echo 'No .wuhb files' && cp -v /project/build/*.elf /host/build/ 2>/dev/null || echo 'No .elf files' && cp -v /project/build/*.rpx /host/build/ 2>/dev/null || echo 'No .rpx files' && cp -v /project/build/game.love /host/build/ 2>/dev/null || echo 'No game.love file' && echo '=== EXTRACTION COMPLETE ==='"

echo.
echo Files extracted to build\ directory:
dir build

echo.
echo Main files:
echo - balatro.wuhb (complete game for Wii U)
echo - lovepotion.wuhb (engine only)
echo - lovepotion.elf (debugging)
echo - lovepotion.rpx (Wii U executable)
echo - game.love (game data)