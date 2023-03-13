@echo off

if not exist ..\build\ mkdir ..\build\

pushd %~dp0\..\build\
cmake ..
popd
pause
