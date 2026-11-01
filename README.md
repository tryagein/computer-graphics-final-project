# OpenGL Scene Editor

Interactive OpenGL scene editor coursework project. See `specs/001-agent-3d-scene/` for the specification and implementation plan. The graphics editor, scene persistence, and Agent scene-operation workflow are implemented.

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

For the existing Visual Studio build in this repository, double-click `run-editor.cmd` in the project root. It starts the built editor from the correct directory and keeps startup error output visible. The DeepSeek key belongs in the ignored `.env` file; see `docs/agent-setup.md`.

The editor starts with a ground plane, cube, point light, and directional light. The top toolbar adds a cube, sphere, or plane and immediately selects it; it can also import an OBJ, add the example model, or reset the camera. The left panel lists objects and toggles visibility. The center is the 3D viewport. The right panel has Properties and Lighting tabs. Click the viewport, hold the right mouse button to look around, use W/A/S/D to move, and Q/E to move vertically. Changes appear immediately in the viewport; status and import errors appear at the bottom. The graphics acceptance steps are in `docs/graphics-acceptance.md`.

### 界面快速上手

1. 点击顶部“立方体”“球体”或“平面”，新对象会出现在 3D 视口中并自动选中。
2. 在左侧“场景对象”中切换选中对象；右侧“属性”可修改位置、旋转、缩放、颜色和纹理。
3. 点击顶部“示例模型”可一键验证 OBJ/MTL 导入；“导入 OBJ”可填写自己的模型路径。
4. 在右侧“灯光”中添加、调整、启用或删除点光源与平行光。
5. 点击视口后，右键拖动观察，W/A/S/D 移动，Q/E 升降；点击“重置相机”返回初始视角。
6. 点击顶部“场景文件”，选择保存文件夹、填写场景名称，然后保存；打开时使用系统文件选择器。移动场景时，请把生成的场景文件和同名 `.resources` 目录一起移动；格式与验收方法见 `docs/scene-format.md`。
7. 打开右侧“Agent”页，可用自然语言生成多步场景修改；模型配置和无需密钥的 JSON 计划验收见 `docs/agent-setup.md`。

The direct graphics editor remains the primary deliverable. Agent features call the same validated scene operations and are optional for direct rendering and editing.
