@echo off
setlocal enabledelayedexpansion

echo ====================================================
echo  RTS Engine - URUCHAMIANIE KLIENTA
echo ====================================================

set "DEFAULT_CHOICE=1"
if exist ".last_preset" set /p DEFAULT_CHOICE=<.last_preset

echo Wybierz zbudowany wariant: [1] Debug  [2] Release  [3] ASan
set /p "CHOICE=Twoj wybor [%DEFAULT_CHOICE%]: "
if "%CHOICE%"=="" set "CHOICE=%DEFAULT_CHOICE%"
echo %CHOICE% > .last_preset

if "%CHOICE%"=="1" ( set "PRESET=windows-msvc-debug" & set "CONF=Debug" )
if "%CHOICE%"=="2" ( set "PRESET=windows-msvc-release" & set "CONF=Release" )
if "%CHOICE%"=="3" ( set "PRESET=windows-msvc-asan" & set "CONF=Debug" )

set "EXE_PATH=build\%PRESET%\build\bin\%CONF%\rts-client.exe"

if not exist "%EXE_PATH%" (
    echo [BLAD] Nie znaleziono "%EXE_PATH%".
    echo Uruchom najpierw skrypt build.bat, aby skompilowac projekt!
    pause
    exit /b 1
)

echo.
echo Uruchamianie gry...
echo.
"%EXE_PATH%"

pause