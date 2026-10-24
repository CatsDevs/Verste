# API Module Audit

This document tracks the modules found in the supplied C++ archive and their status in Verste.

Verste is a Windows Luau editor. It does not attach to, inject into, or tamper with the Roblox client. This inventory is for source review and compatibility planning; listing a symbol does not mean Verste implements it.

## Archive inventory

| Module | Files | Review notes |
|---|---|---|
| actor | `actor.cpp/.hpp` | Runtime integration; depends on the source project's execution model |
| bit | `bit.cpp/.hpp` | Potentially portable utility; review before adoption |
| cache | `cache.cpp/.hpp` | Runtime-specific state and lifetime assumptions |
| closures | `closures.cpp/.hpp` | Function hooking and executor closure behavior; not ported |
| crypt | `crypt.cpp/.hpp`, `Sha3.hpp` | Cryptographic helpers; license and dependency review required |
| debug | `debug.cpp/.hpp` | Runtime introspection; API differs by VM |
| drawing | `drawing.cpp/.hpp` | Executor drawing API; not ported |
| drawingimmediate | `drawingimmediate.cpp/.hpp` | Renderer-specific drawing implementation |
| filedialog | `filedialog.cpp/.hpp` | Potentially reusable UI integration; check Qt/Win32 assumptions |
| filesystem | `filesystem.cpp/.hpp` | File API; must be scoped to an explicit workspace |
| http | `http.cpp/.hpp` | Network API; must use explicit allowlists and timeouts if ever enabled |
| input | `input.cpp/.hpp` | Synthetic input API; not ported |
| instance | `instance.cpp/.hpp` | Roblox object model integration; not available in the editor |
| luastateproxy | `luastateproxy.cpp/.hpp` | VM internals and state ownership assumptions |
| metatable | `metatable.cpp/.hpp` | Runtime metatable interception; not ported |
| miscellaneous | `miscellaneous.cpp/.hpp` | Mixed helpers; review symbol-by-symbol |
| oth | `oth.cpp/.hpp` | Unclassified helpers; review symbol-by-symbol |
| raknet | `raknet.cpp/.hpp` | Roblox/network protocol integration; not ported |
| rconsole | `rconsole.cpp/.hpp` | Console helpers; could be adapted to Verste's own output panel |
| reflection | `reflection.cpp/.hpp` | Runtime reflection assumptions |
| regex | `regex.cpp/.hpp` | Potentially portable utility; review dependency and license |
| script | `script.cpp/.hpp` | Script execution and runtime integration |
| signals | `signals.cpp/.hpp` | Runtime event connections; not ported |
| websocket | `websocket.cpp/.hpp` | Network API; requires permission and lifecycle review |

## API compatibility checklist

The following names are commonly seen in executor-specific APIs. They are **not** implemented by this checklist.

### Environment and execution
- `identifyexecutor`, `getexecutorname`, `getexecutorversion`
- `getgenv`, `getrenv`, `getreg`, `getgc`
- `loadstring`, `checkcaller`, `getcallingscript`

### Filesystem
- `readfile`, `writefile`, `appendfile`, `listfiles`
- `isfile`, `isfolder`, `makefolder`, `delfile`, `delfolder`
- `loadfile`, `dofile`

### Closures and debugging
- `newcclosure`, `iscclosure`, `islclosure`, `clonefunction`
- `hookfunction`, `restorefunction`, `checkclosure`
- `debug.getinfo`, `debug.getupvalue`, `debug.getupvalues`
- `debug.getconstant`, `debug.getconstants`, `debug.setconstant`
- `debug.getstack`, `debug.setstack`

### Metatables and runtime objects
- `getrawmetatable`, `setrawmetatable`, `setreadonly`, `isreadonly`
- `getnamecallmethod`, `setnamecallmethod`
- `getinstances`, `getnilinstances`, `getloadedmodules`
- `gethui`, `getcustomasset`, `cloneref`, `compareinstances`
- `gethiddenproperty`, `sethiddenproperty`, `isscriptable`

### Drawing, signals, input, and networking
- `Drawing.new`, `Drawing.clear`
- `getconnections`, `firesignal`
- `fireclickdetector`, `fireproximityprompt`, `firetouchinterest`
- `setclipboard`, `keypress`, `keyrelease`, `mouse1click`
- `request`, `http_request`, WebSocket operations
- `crypt.base64encode`, `crypt.base64decode`, `crypt.hash`

## Dependency gaps observed in the supplied archive

The archive references project-specific headers and runtime facilities (including RBX execution/environment headers, global utility headers, Luau, and ImGui). Those dependencies were not part of the supplied archive, so the files cannot be dropped into Verste and expected to compile.

## Porting policy

1. Review licensing and provenance for every source file before copying it.
2. Port only self-contained, general-purpose utilities after dependency and security review.
3. Keep file operations constrained to a user-selected workspace.
4. Keep any script execution isolated from the UI process and disable ambient network/filesystem access by default.
5. Do not add Roblox-process injection, memory tampering, exploit hooks, synthetic gameplay input, or anti-cheat bypasses to Verste.
6. Track each accepted utility with a unit test and a documented public interface.
