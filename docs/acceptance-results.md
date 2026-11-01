# Phase 6 quickstart acceptance results

Executed on 2026-11-01 (Asia/Shanghai) against the Visual Studio 2022 Debug build. Target machine, GPU, driver, resolution, and the 60-second measurement are in [performance-results.md](performance-results.md). The rendered demo image is [course-demo-preview.png](course-demo-preview.png). Commands and setup are in the project README.

| Quickstart scenario | Outcome and evidence |
| --- | --- |
| Build and launch | **Pass.** `scene_editor`, `scene_agent_tests`, `scene_persistence_tests`, and `generate_course_demo` build. The editor renders the saved demo at OpenGL 3.3 without shader/context errors. |
| Cube, sphere, plane; independent transforms | **Pass for scene data and rendering.** The demo contains all primitive types; the rendered preview shows them. Agent operation tests add and modify objects by stable ID and reject invalid transforms without changing the object. Earlier direct-edit workflow was accepted by the user. |
| Free camera movement | **Pass for camera/render path.** The 60-second editor benchmark continuously moves and turns the camera. The regular editor exposes right-mouse look and WASD/QE movement; this run did not inject physical keyboard/mouse input. |
| OBJ geometry, normals, UVs, MTL, texture; transform | **Pass.** The CC0 fixture imports through Assimp; automated checks confirm nonempty geometry, normals, UVs, MTL texture reference, and a real PNG. A direct import operation and scale update pass. The rendered preview shows the checker texture on the crate. |
| Point and directional lights; color/intensity | **Pass.** The demo has both light types; the preview shows cool overall and warm local illumination. Automated operation checks update point-light color/intensity and add a light. |
| Object material and texture changes | **Pass.** `set_material` tests update the targeted object without changing unrelated object transforms. The demo renders an MTL diffuse texture. |
| 100 objects for 60 seconds while navigating | **Pass.** Full editor and scripted camera orbit: 60.0103 seconds, median 60.0017 FPS at a 628×580 viewport; target ≥30 FPS. See performance report. |
| Save and inspect metadata/resources | **Pass.** Persistence tests write versioned JSON and sibling `.resources` directories with OBJ, MTL, and texture dependencies. The committed demo is a real saved scene package. |
| Move scene directory and reopen | **Pass.** Persistence tests move a saved package and reopen it with IDs, materials, transforms, lights, camera, and resource paths intact. |
| Malformed scene and missing resource | **Pass.** Automated tests reject both while preserving the active scene and camera. |
| Natural-language snowy night scene | **Pass in the provider/scene-operation path.** With the user's configured DeepSeek key, a real response produced three primitive trees, street-lamp geometry, a warm point light (new or reused), and camera framing; the plan executed without an operation error. The output was checked against the scene state. |
| Targeted Agent follow-up | **Pass.** A real follow-up request changed one stable-ID object's base color to blue. Serialized unrelated objects, lights, and camera remained identical. |
| Missing Agent target | **Pass.** A real request for a nonexistent object left the serialized scene unchanged; the operation executor's missing-target unit test also confirms state-query feedback and stop-on-error behavior. |
| Agent unavailable | **Pass for the service boundary.** An unconfigured provider reports failure cleanly; direct scene operations, rendering, and persistence tests run without calling the provider. |

`ctest --test-dir build-vs -C Debug --output-on-failure` passed 2/2 tests after these checks. The live Agent scenario is opt-in and uses a real API request; run it with `DEEPSEEK_LIVE_TEST=1` and `DEEPSEEK_LIVE_SCENARIO=1` as described in [agent-setup.md](agent-setup.md).

The window controls, file picker, and Agent progress panel were not automated by this run; they remain the final interactive visual check when presenting the course demo. The earlier graphics and persistence workflows were user-accepted in this project conversation. This representative quickstart does not claim the larger 10-file OBJ, 20-request Agent, or 10-trial persistence sample sizes in the specification's statistical success criteria.
