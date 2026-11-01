# Research: OpenGL 3D Scene Editor with AI Agent

## Decisions

### Native application baseline

- **Decision**: Plan for C++17, CMake, and an OpenGL 3.3 Core renderer on Windows as the initial target.
- **Rationale**: This is a conventional desktop OpenGL baseline that supports programmable shaders and matches the course's explicit OpenGL focus. C++17 is a planning assumption because the repository currently contains no application source or build configuration; confirm it against the course environment before implementation.
- **Alternatives considered**: A higher OpenGL version may narrow available hardware; a cross-platform web renderer would weaken the native OpenGL coursework emphasis.

### Window, input, and context

- **Decision**: Use GLFW for window creation, OpenGL context setup, and keyboard/mouse input; use GLAD for OpenGL entry-point loading.
- **Rationale**: GLFW provides platform-independent window, context, and input APIs and documents the required current-context lifecycle. Keeping it limited to platform/input responsibilities avoids coupling scene logic to windowing. The official GLFW documentation describes these APIs and context constraints: https://www.glfw.org/docs/latest/ and https://www.glfw.org/docs/latest/context_guide.html.
- **Alternatives considered**: Native Win32 windowing would add platform-specific code; a different windowing library is possible if required by the course lab image.

### Math, model import, UI, and serialization

- **Decision**: Use GLM for vectors/matrices, Assimp for OBJ/MTL import, Dear ImGui for editor panels, and nlohmann/json for scene and operation serialization, subject to dependency/license compatibility review.
- **Rationale**: These choices cover common desktop graphics coursework needs without requiring custom matrix math, mesh parsing, or immediate-mode editor widgets. Keep adapters around import and JSON boundaries so domain data does not depend on third-party types.
- **Alternatives considered**: Hand-written math/UI/import code is possible but increases risk and distracts from demonstrating rendering; other libraries may be substituted if available in the course environment.

### Scene persistence and asset portability

- **Decision**: Store scene metadata in a versioned JSON document and copy imported model/material/texture files into a sibling resource directory, referenced by relative paths.
- **Rationale**: This follows the clarified requirement and allows a scene directory to move as a unit. Validate schema version, entities, and resource references before replacing the active scene.
- **Alternatives considered**: Absolute source paths are fragile across machines; packing all assets into a custom binary container is unnecessary for the baseline.

### Agent integration

- **Decision**: Define a provider-independent operation schema and dispatcher. Keep model/provider choice behind an adapter; require strict argument validation and return operation results plus a compact scene snapshot.
- **Rationale**: Provider credentials/network failure must not enter the renderer or disable direct editing. The user explicitly requires structured calls and no generated OpenGL code. Do not choose a provider until account access, cost, and course constraints are known.
- **Alternatives considered**: Direct model-generated code execution is excluded by the spec; hard-coding a provider into the scene core creates unnecessary coupling.

### Test approach

- **Decision**: Use CMake build validation, deterministic unit tests for scene math/validation/serialization/dispatch, integration checks for asset import and scene operation paths, plus a repeatable manual visual checklist.
- **Rationale**: Renderer appearance needs human-visible acceptance while most state transformations can be tested deterministically. Framework choice can follow the toolchain already available on the target machine.
- **Alternatives considered**: Screenshot-only checks are brittle and insufficient for state correctness; unit tests alone cannot validate visible lighting and camera behavior.

## Open Setup Decisions

- Confirm compiler/IDE and C++ standard with the course machine before dependency installation.
- Confirm whether the Agent may call a hosted service; if so, document credential handling and network failure behavior without putting secrets in scene files or source control.
- Record GPU/driver, viewport resolution, and exact 100-object test scene for reproducible performance measurement.
- Verify dependency licenses and versions when pinning packages.
