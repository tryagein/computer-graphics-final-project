# Dependencies

Initial dependency set in `cmake/Dependencies.cmake`:

| Dependency | Version | Purpose | License |
|---|---:|---|---|
| GLFW | 3.4 | Window, context, input | zlib/libpng |
| GLAD | 2.0.8 | OpenGL function loading | MIT |
| GLM | 1.0.1 | Vector/matrix math | MIT |
| Assimp | 5.4.3 | OBJ/MTL import | BSD-3-Clause |
| Dear ImGui | 1.91.8 | Editor UI | MIT |
| nlohmann/json | 3.11.3 | JSON serialization | MIT |
| stb_image | 003501f6d7a8668ce8b6d875e34dd97ad0d5d5b5 | Texture decoding | Public domain or MIT |

Sources are the upstream repositories pinned in `cmake/Dependencies.cmake` and fetched by CMake with `SCENE_EDITOR_FETCH_DEPS=ON`; no third-party source is committed. Verify upstream license texts before distribution. OpenGL is supplied by the platform driver.
