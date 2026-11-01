# OpenGL Scene Editor

Interactive OpenGL scene editor coursework project. See `specs/001-agent-3d-scene/` for the specification and implementation plan. T001–T026 provide the graphics editor foundation; later tasks cover scene persistence and Agent operations.

## Requirements

- Windows 10/11 and GPU driver with OpenGL 3.3 Core support
- CMake 3.24+, C++17 toolchain (Visual Studio 2022 with Desktop development with C++ recommended)
- Network access on first configure when dependency fetching is enabled

## Configure and build

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DSCENE_EDITOR_FETCH_DEPS=ON
cmake --build build --config Release --target scene_editor --parallel 4
```

Run `build/Release/scene_editor.exe` with Visual Studio generators or `build/scene_editor.exe` with single-configuration generators. Build from CLion by opening this directory as a CMake project, selecting a Visual Studio or MinGW toolchain, reloading CMake, then running the `scene_editor` target. CLion's bundled CMake can be used if `cmake` is not on `PATH`. The first configure downloads dependencies; if your network requires an HTTP proxy, configure Git's proxy before reloading CMake. See `DEPENDENCIES.md` for versions and provenance.

The editor starts with a ground plane, cube, point light, and directional light. Use the Objects, Material, and Lights panels to edit them. The 3D view fills the window behind the panels. Hold the right mouse button over the 3D view to look around; W/A/S/D move, and Q/E move vertically. Import `assets/examples/pyramid.obj` through the Objects panel to try OBJ/MTL loading. The graphics acceptance steps are in `docs/graphics-acceptance.md`.

The direct graphics editor is the primary deliverable. Agent features are layered on the scene-operation contract and are not required for direct rendering and editing.
