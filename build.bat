@echo off
setlocal enabledelayedexpansion

echo ====================================================
echo  RTS Engine - 2. BUILD (CMake)
echo ====================================================

set "DEFAULT_CHOICE=1"
if exist ".last_preset" set /p DEFAULT_CHOICE=<.last_preset

echo Wybierz profil do zbudowania: [1] Debug  [2] Release  [3] ASan
set /p "CHOICE=Twoj wybor [%DEFAULT_CHOICE%]: "
if "%CHOICE%"=="" set "CHOICE=%DEFAULT_CHOICE%"
echo %CHOICE% > .last_preset

if "%CHOICE%"=="1" set "PRESET=windows-msvc-debug"
if "%CHOICE%"=="2" set "PRESET=windows-msvc-release"
if "%CHOICE%"=="3" set "PRESET=windows-msvc-asan"

if not exist "build\%PRESET%\build\generators\conan_toolchain.cmake" (
    echo [BLAD] Brak zaleznosci. Uruchom najpierw setup.bat dla tego profilu!
    pause
    exit /b 1
)

echo.
echo [1/2] Konfiguracja CMake...
cmake --preset %PRESET%
if errorlevel 1 ( echo [BLAD] Konfiguracja CMake nie powiodla sie. & pause & exit /b 1 )

echo.
echo [2/2] Kompilacja kodu...
cmake --build --preset %PRESET%
if errorlevel 1 ( echo [BLAD] Budowanie kodu nie powiodlo sie. & pause & exit /b 1 )

echo.
echo ====================================================
echo  Build zakonczony sukcesem!
echo ====================================================
pause