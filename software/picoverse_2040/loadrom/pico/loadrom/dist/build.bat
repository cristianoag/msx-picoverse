@echo off
if "%1"=="" (
    echo "Please provide the ROM file name as a parameter."
    echo "Usage: %0 <ROM_FILE_NAME>"
    exit /b
)

set ROM_FILE=%1
copy /b loadrom.bin+%ROM_FILE% resultant.bin
uf2conv.exe resultant.bin resultant.uf2
copy resultant.uf2 d:\
