@echo off

if not exist "P:\8086\bin\" mkdir "P:\8086\bin"

pushd .\bin

cl -Zi ..\src\eightysix.c /DDEBUG

popd .\bin
