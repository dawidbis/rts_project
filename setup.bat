@echo off
setlocal enabledelayedexpansion

echo ====================================================
echo  RTS Engine - 1. SETUP (Conan)
echo ====================================================

set "DEFAULT_CHOICE=1"
if exist ".last_preset" set /p DEFAULT_CHOICE=<.last_preset

echo Wybierz profil:
echo   [1] Debug (Domyslny, do codziennej pracy)
echo   [2] Release (Zoptymalizowany, bez debuggera)
echo   [3] ASan (Wymaga dluuuugiej kompilacji zaleznosci ze zrodel!)
echo.
set /p "CHOICE=Twoj wybor [%DEFAULT_CHOICE%]: "
if "%CHOICE%"=="" set "CHOICE=%DEFAULT_CHOICE%"
echo %CHOICE% > .last_preset

if "%CHOICE%"=="1" (
    set "PRESET=windows-msvc-debug"
    set "BUILD_TYPE=Debug"
) else if "%CHOICE%"=="2" (
    set "PRESET=windows-msvc-release"
    set "BUILD_TYPE=Release"
) else if "%CHOICE%"=="3" (
    set "PRESET=windows-msvc-asan"
    set "BUILD_TYPE=Debug"
) else (
    echo [BLAD] Nieprawidlowy wybor.
    pause
    exit /b 1
)

set "BUILD_DIR=build\%PRESET%"
echo.
echo [1/3] Sprawdzanie srodowiska...
python --version >nul 2>&1
if errorlevel 1 ( echo [BLAD] Python nie jest zainstalowany. & pause & exit /b 1 )
conan --version >nul 2>&1
if errorlevel 1 ( echo [BLAD] Conan nie jest zainstalowany. & pause & exit /b 1 )

echo [2/3] Konfigurowanie profilu Conana...
conan profile detect --force >nul 2>&1

echo [3/3] Instalowanie zaleznosci dla profilu: %PRESET%...
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

if "%CHOICE%"=="3" (
    echo [UWAGA] Kompilacja zaleznosci dla ASan potrwa kilkanascie/kilkadziesiat minut...
    conan install . --build=* -s build_type=%BUILD_TYPE% -s compiler.cppstd=23 -c "tools.build:cxxflags+=/fsanitize=address" -c "tools.build:cflags+=/fsanitize=address" -c "tools.build:sharedlinkflags+=/fsanitize=address" -c "tools.build:exelinkflags+=/fsanitize=address" --output-folder=%BUILD_DIR%
) else (
    conan install . --build=missing -s build_type=%BUILD_TYPE% -s compiler.cppstd=23 --output-folder=%BUILD_DIR%
)

if errorlevel 1 ( echo. & echo [BLAD] Conan install nie powiodl sie. & pause & exit /b 1 )

echo.
echo ====================================================
echo  Setup zakonczony! Mozesz teraz uruchomic build.bat
echo ====================================================
pause