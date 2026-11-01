# Phase 7: richer OBJ assets and directional shadows

## Built-in assets

The toolbar's **预设模型** menu offers three original OBJ/MTL assets:

| Asset ID | Scene result |
| --- | --- |
| `snow_pine` | A tapered trunk, four foliage tiers, and four snow caps in one multi-material OBJ. |
| `street_lamp` | A metal post, arm, brass lantern, emissive glass, and a colocated warm point light. |
| `snow_cabin` | Timber walls, a pitched snow roof, a door, framed windows, and emissive glass. |

The asset source is `tools/generate_prefab_assets.py`; generated files, a subtle `snow-grain.png` ground texture, and permission text are in `assets/prefabs/`. Each `add_prefab` operation accepts `asset_id`, optional `id`, and optional `[x,y,z]` `position`. It resolves a fixed, bundled file path, verifies the OBJ, then adds scene objects. An invalid ID, missing asset, duplicate object ID, or point-light capacity error leaves the scene unchanged. Scene saving copies OBJ/MTL/PNG dependencies into the scene's `.resources` directory, so the resulting JSON can be moved with its resources.

The lamp's point light is named `<object-id>-light`. The lamp model and light are separate scene entities: when moving the lamp later, move its light to the matching position if the glow should follow. The imported `Ke` glass color is rendered as an emissive material, while the point light illuminates nearby surfaces. Importing a custom OBJ with MTL `Ke` also displays emissive material.

When several objects reference the same source OBJ, the scene packager copies it once and points each object at the shared packaged path. The renderer then reuses one uploaded mesh cache entry for that path.

## Shadow pipeline

When enabled, the renderer draws visible primitives and imported OBJ submeshes into a 2048×2048 depth texture from the first enabled directional light. The main Blinn–Phong pass samples the depth texture with percentage-closer filtering (PCF). The **灯光** panel offers a switch, a depth-bias slider, and a PCF radius of 0, 1, or 2 (1, 9, or 25 depth samples). Ambient and emissive terms remain visible in shadow. A scene with no active directional light skips the depth pass.

The shadow camera currently covers a fixed 40×40 world-unit area centered at the origin, with a depth range of 1–75 units. Geometry far outside that area receives direct light without a shadow. Only one directional light casts shadows; point lights do not. This is a practical first shadow-mapping pass for the course scene, not cascaded shadow mapping or physically based rendering. The shadow switch and bias are editor settings and are not stored in the scene JSON.

## Reproduce and inspect

```powershell
python tools/generate_prefab_assets.py
cmake --build build-vs --config Debug --target generate_course_demo scene_editor scene_agent_tests scene_persistence_tests --parallel 4
& '.\build-vs\Debug\generate_course_demo.exe'
ctest --test-dir build-vs -C Debug --output-on-failure
& '.\build-vs\Debug\scene_editor.exe' --capture-demo '.\build-vs\with-shadows.png'
& '.\build-vs\Debug\scene_editor.exe' --capture-demo-no-shadows '.\build-vs\without-shadows.png'
```

Open `assets/examples/course-demo/course-demo.json` to inspect the scene interactively. Toggle shadows in **灯光**, then inspect the tree and lamp shadows on the ground. Add a prefab from the toolbar or ask the Agent: `添加三棵积雪松树和一盏暖光路灯，让摄像机面向路灯。`
