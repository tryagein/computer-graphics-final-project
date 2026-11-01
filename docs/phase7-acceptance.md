# Phase 7 graphics acceptance

Executed on 2026-11-01 (Asia/Shanghai) with the Visual Studio 2022 Debug build and NVIDIA RTX 3050 Laptop GPU.

| Check | Result |
| --- | --- |
| Built-in OBJ assets | Pass. Three `snow_pine`, one `street_lamp`, and one `snow_cabin` are imported in the saved course demo. The lamp glass and cabin window use MTL `Ke`; the ground and crate use packaged PNG textures. |
| Portable scene | Pass. `generate_course_demo` saved and reloaded the 7-object, 2-light scene. Three trees share one packaged OBJ path. Agent and persistence tests passed 2/2 after the change. |
| Agent asset catalog | Pass. Offline tests cover adding all three IDs, duplicate-ID rejection, and emissive lamp MTL import. A live DeepSeek snowy-night request produced four new prefab objects and one light, then a targeted edit and missing-target request preserved unrelated state. |
| Directional shadows | Pass. The OpenGL capture with shadows shows tree and lamp silhouettes on the ground. With shadows disabled, those silhouettes disappear. Across a 2-pixel grid in the central scene region, 2,881 samples changed by over 30 summed RGB values; all 2,881 were darker with shadows enabled. |
| Performance | Pass. The fixed 1280×720 offscreen benchmarks measured 143.995 FPS for 100 primitives over 60 seconds and 144.003 FPS for 70 primitives plus 30 OBJ prefabs over 30 seconds. See [performance-results.md](performance-results.md). |

Images: [shadows enabled](course-demo-preview.png), [shadows disabled](course-demo-no-shadows.png). The toolbar and Lighting panel controls have not been exercised by automated pointer input; open the demo in the editor for final manual interaction checks.
