# cache_simulator
A C++ cache simulator with a native wxWidgets graphical interface. The project allows users to configure cache size, block size, associativity, and replacement policy, then run memory trace simulations step by step or all at once. It displays hits, misses, evictions, an execution log, and a visual representation of cache sets and ways.

## Requirements

- Visual Studio C++ compiler
- CMake
- vcpkg
- wxWidgets installed through vcpkg

Install wxWidgets if it is not already installed:

```powershell
cd "C:\Users\faraz\Desktop\vcpkg"
.\vcpkg install wxwidgets:x64-windows
```

## Compile

Open PowerShell and run:

```powershell
cd "C:\Users\faraz\Desktop\cache_simulator_imgui_no_comments\cache_simulator_imgui"

cmd /d /c 'call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" && cmake -S . -B build -G "NMake Makefiles" -DCMAKE_TOOLCHAIN_FILE="C:/Users/faraz/Desktop/vcpkg/scripts/buildsystems/vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-windows && cmake --build build'
```

## Run

After compiling, run:

```powershell
.\build\cache_simulator.exe
```

## Using The Interface

1. Enter the cache settings:
   - Total Cache Size
   - Block Size
   - Associativity

2. Select a replacement policy:
   - LRU
   - FIFO
   - Random

3. Enter trace instructions in the trace editor, or click `Load Trace` to open a trace file.

4. Use the controls:
   - `Run Simulation` runs the full trace.
   - `Step` runs one trace instruction at a time.
   - `Reset` clears the current simulation and reloads the settings.

5. Read the output:
   - The stats line shows hits, misses, and evictions.
   - The execution log shows each memory access result.
   - The cache visualizer shows sets and ways. The most recently accessed line is highlighted.

## Trace Format

Each trace line should contain an operation and a hexadecimal address:

```text
R 0x0000
W 0x0040
R 0x0080
```

`R` means read and `W` means write.
