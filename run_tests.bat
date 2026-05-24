@echo off
setlocal enabledelayedexpansion

echo ====================================================
echo  RTS Engine - 3. TESTY (CTest)
echo ====================================================

set "DEFAULT_PRESET=1"
if exist ".last_preset" set /p DEFAULT_PRESET=<.last_preset

echo Wybierz profil: [1] Debug  [2] Release  [3] ASan
set /p "PRESET_CHOICE=Twoj wybor [%DEFAULT_PRESET%]: "
if "%PRESET_CHOICE%"=="" set "PRESET_CHOICE=%DEFAULT_PRESET%"
echo %PRESET_CHOICE% > .last_preset

if "%PRESET_CHOICE%"=="1" set "PRESET=windows-msvc-debug"
if "%PRESET_CHOICE%"=="2" set "PRESET=windows-msvc-release"
if "%PRESET_CHOICE%"=="3" set "PRESET=windows-msvc-asan"

echo.
echo Jakie testy chcesz uruchomic?
echo   [1] Wszystkie (Client, Server, Common)
echo   [2] Tylko Klient (test_client)
echo   [3] Tylko Serwer (test_server)
echo   [4] Tylko Common (test_common)
echo.
set /p "TEST_CHOICE=Twoj wybor [1]: "
if "%TEST_CHOICE%"=="" set "TEST_CHOICE=1"

set "CTEST_ARGS="
if "%TEST_CHOICE%"=="2" set "CTEST_ARGS=-R client"
if "%TEST_CHOICE%"=="3" set "CTEST_ARGS=-R server"
if "%TEST_CHOICE%"=="4" set "CTEST_ARGS=-R common"

echo.
echo Uruchamianie testow...
echo Komenda: ctest --preset %PRESET% %CTEST_ARGS%
echo.

ctest --preset %PRESET% %CTEST_ARGS% --output-on-failure

if errorlevel 1 (
    echo.
    echo [BLAD] Przynajmniej jeden test zakonczyl sie niepowodzeniem!
    pause
    exit /b 1
)

echo.
echo ====================================================
echo  Wybrane testy zaliczone pomyslnie!
echo ====================================================
pause