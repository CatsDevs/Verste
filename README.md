# Verste

Verste is a dark-themed Windows desktop editor for Luau, built with C++17 and Qt 6.

## Features
- Dark editor UI inspired by script editors
- Multiple tabs with new/open/save/save-as and close controls
- Luau syntax highlighting
- Line numbers
- Find and replace
- Basic Luau keyword autocomplete
- Static checks and diagnostics in a console
- No Roblox process attachment, injection, exploit API implementation, or client tampering

## Requirements
- Windows 10/11
- Visual Studio 2022 with **Desktop development with C++**
- Qt 6.4+ with the MSVC 64-bit kit
- CMake 3.21+

## Build
Open **x64 Native Tools Command Prompt for VS 2022**:

```bat
cmake -S . -B build -DCMAKE_PREFIX_PATH=C:\Qt\6.x.x\msvc2022_64
cmake --build build --config Release
```

Replace the Qt path with the path to your Qt installation. The executable will be under `build\Release\Verste.exe`.

## Test mode
The current test action performs static checks only. It is not a Luau VM and does not execute scripts. For real execution, integrate the official Luau VM in a separate, restricted worker process with CPU/memory limits and no network/filesystem privileges. Do not execute untrusted scripts inside the UI process.
