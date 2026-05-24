# RTS Engine

Silnik gry RTS czasu rzeczywistego oparty na architekturze typu *authoritative server*.
Inspirowany tytulami openfront.io / territorial.io, rozszerzony o autorskie mechaniki rozgrywki.

## Struktura projektu

```
rts_project/
├── CMakeLists.txt          # Glowna konfiguracja budowania
├── CMakePresets.json       # Presety dla IDE / CLI (VS, Linux, CI)
├── conanfile.py            # Definicje zaleznosci (Conan 2)
├── setup.bat               # Skrypt konfiguracyjny dla Windows
├── setup.sh                # Skrypt konfiguracyjny dla Linux
├── .clang-tidy             # Reguly analizy statycznej
├── .clang-format           # Styl kodu
│
├── cmake/                  # Reuzywalne moduly CMake
│   ├── CompilerWarnings.cmake
│   ├── Sanitizers.cmake
│   └── StaticAnalysis.cmake
│
├── common/                 # Biblioteka wspoldzielona (protokol, ECS, math, utils)
│   ├── CMakeLists.txt
│   ├── include/
│   └── src/
│
├── server/                 # Serwer autorytatywny gry
│   ├── CMakeLists.txt
│   ├── include/
│   └── src/
│
├── client/                 # Klient gry (SFML + Dear ImGui)
│   ├── CMakeLists.txt
│   ├── include/
│   └── src/
│
├── tests/                  # Testy jednostkowe (Catch2)
│   ├── common/
│   ├── server/
│   └── client/
│
├── docker/                 # Definicje kontenerow i pliki compose
└── .github/workflows/      # Pipeline'y CI/CD
```

## Wymagania wstepne

- **CMake** w wersji 3.23 lub nowszej
- **Python** 3.10+ (potrzebny dla Conana)
- **Conan 2.x** (`pip install --upgrade "conan>=2.0"`)
- **Kompilator C++ ze wsparciem dla C++23**:
    - Windows: Visual Studio 2022 (17.5+) z komponentem *Desktop development with C++*
    - Linux: GCC 13+ lub Clang 16+

## Szybki start - skrypty pomocnicze

Dla wygody w glownym katalogu projektu znajduja sie skrypty automatyzujace pierwsza konfiguracje (wykrycie profilu Conana + pobranie zaleznosci):

### Windows

```powershell
.\setup.bat
```

Skrypt:
1. Sprawdza obecnosc Pythona i Conana.
2. Wykrywa domyslny profil Conana (`conan profile detect --force`).
3. Pobiera i buduje zaleznosci do folderu `build/windows-msvc-debug/`.

Po jego zakonczeniu mozna przejsc do dalszej kompilacji lub otworzyc projekt w Visual Studio.

### Linux

```bash
chmod +x setup.sh
./setup.sh
```

Skrypt wykonuje analogiczne kroki, pobierajac zaleznosci do folderu `build/linux-gcc-debug/`.

## Recznie - pierwsze uruchomienie

Jezeli wolisz wykonac kroki recznie zamiast przez skrypt, raz na srodowisko nalezy wykryc domyslny profil Conana:

```bash
conan profile detect --force
```

## Budowanie (Windows / Visual Studio 2022)

```powershell
# 1. Pobranie zaleznosci (Debug)
conan install . --build=missing -s build_type=Debug `
    --output-folder=build/windows-msvc-debug

# 2. Konfiguracja i budowanie
cmake --preset windows-msvc-debug
cmake --build --preset windows-msvc-debug

# 3. Uruchomienie testow
ctest --preset windows-msvc-debug
```

Dla wersji Release wystarczy zamienic `Debug` -> `Release` oraz preset `windows-msvc-debug` -> `windows-msvc-release`.

Przy pierwszym uruchomieniu czesc bibliotek (m.in. `boost`, `libpqxx`, `sfml`) moze byc budowana ze zrodel - moze to potrwac kilkanascie minut i zajac kilka GB miejsca na dysku.

### Otwieranie w Visual Studio

1. Najpierw wykonaj krok `conan install` (recznie lub przez `setup.bat`).
2. W VS: **File -> Open -> CMake...** i wskaz plik `CMakeLists.txt` projektu.
3. Visual Studio automatycznie wczyta `CMakePresets.json`. **Wazne:** w gornym pasku konfiguracji wybierz preset **`windows-msvc-debug`** - VS domyslnie moze proponowac wlasny preset `x64-Debug`, ktory nie korzysta z toolchainu Conana i spowoduje bledy typu "Could not find package configuration file".

## Budowanie (Linux)

```bash
conan install . --build=missing -s build_type=Debug \
    --output-folder=build/linux-gcc-debug
cmake --preset linux-gcc-debug
cmake --build --preset linux-gcc-debug -j
ctest --preset linux-gcc-debug
```

## Opcje budowania

| Opcja CMake               | Domyslnie | Opis                                            |
|---------------------------|-----------|-------------------------------------------------|
| `RTS_BUILD_SERVER`        | ON        | Buduj plik wykonywalny serwera                  |
| `RTS_BUILD_CLIENT`        | ON        | Buduj plik wykonywalny klienta                  |
| `RTS_BUILD_TESTS`         | ON        | Buduj testy jednostkowe (Catch2)                |
| `RTS_ENABLE_CLANG_TIDY`   | OFF       | Uruchamiaj clang-tidy podczas kompilacji        |
| `RTS_ENABLE_ASAN`         | OFF       | AddressSanitizer                                |
| `RTS_ENABLE_UBSAN`        | OFF       | UndefinedBehaviorSanitizer                      |
| `RTS_ENABLE_TSAN`         | OFF       | ThreadSanitizer (wzajemnie wyklucza sie z ASan) |
| `RTS_WARNINGS_AS_ERRORS`  | OFF       | Traktuj ostrzezenia kompilatora jako bledy      |

Opcje nadpisuje sie przy konfiguracji, np. `-DRTS_ENABLE_ASAN=ON`.

## Rozwiazywanie problemow

- **"Python is not recognized"** - upewnij sie, ze Python jest dodany do zmiennej srodowiskowej `PATH`.
- **"Conan not found"** - uruchom `pip install conan` w terminalu.
- **"Could not find package configuration file provided by EnTT/Boost/..."** - klasyczny objaw odpalenia CMake bez `conan install`. Wykonaj najpierw skrypt `setup.bat`/`setup.sh` lub recznie `conan install`, a w Visual Studio upewnij sie, ze wybrany jest preset z `CMakePresets.json` (np. `windows-msvc-debug`), a nie domyslny VS-owy `x64-Debug`.
- **CMake nie widzi presetow** - sprawdz czy plik `CMakePresets.json` znajduje sie w glownym katalogu projektu i czy uzywasz CMake 3.23+.
- **Blad przy budowaniu bibliotek** - jezeli proces zatrzyma sie przy bibliotece (np. `libpqxx`), upewnij sie, ze masz najnowsze sterowniki kompilatora i wystarczajaca ilosc miejsca na dysku (budowanie ze zrodel moze zajac kilka GB).

## Dodatkowe informacje

Projekt wykorzystuje **Conan 2.0** do zarzadzania zaleznosciami. Plik `conanfile.py` zawiera pelna liste bibliotek. Jezeli potrzebujesz dodac nowa biblioteke:

1. dopisz ja w metodzie `requirements` w pliku `conanfile.py`,
2. dodaj odpowiedni `find_package(...)` w glownym `CMakeLists.txt`,
3. zlinkuj nowy target w odpowiednim podprojekcie (`common`, `server` lub `client`),
4. ponownie uruchom `setup.bat`/`setup.sh` (lub recznie `conan install`) oraz konfiguracje CMake.