# Quickstart: Validation Guide

This guide defines end-to-end checks for the coursework baseline. Exact build commands depend on the selected compiler and dependency setup; record those commands in the project README when implementation setup is complete.

## Prerequisites

- Windows development machine with a GPU/driver that supports the selected OpenGL 3.3 Core baseline.
- C++17-capable compiler and CMake, with GLFW, GLAD, GLM, Assimp, Dear ImGui, and nlohmann/json available through the documented dependency setup.
- A supported OBJ fixture with positions, normals, texture coordinates, an MTL file, and a referenced texture.
- Agent provider configuration only for natural-language scenarios; direct graphics checks must not depend on provider credentials.

## Build and Launch

1. Configure and build using the project CMake preset or documented IDE configuration.
2. Launch the editor and confirm the viewport opens without shader or context errors.
3. Record compiler, dependency versions, GPU/driver, and viewport size used for acceptance runs.

## Baseline Graphics Scenarios

1. Add a cube, sphere, and plane. Verify all render and can be selected/transformed independently.
2. Navigate with the free camera using the documented keyboard/mouse controls; verify view direction and movement respond smoothly.
3. Import the OBJ fixture. Verify geometry, normals, texture coordinates, MTL material, and texture are visible; move and scale it.
4. Add one point light and one directional light. Change each light's color/intensity and verify the expected visible contribution changes.
5. Change an object's color and specular response or texture. Verify the material update appears without changing unrelated objects.
6. Run the specified 100-object scene for 60 seconds while navigating. Record median frame rate and target machine details; acceptance target is at least 30 FPS.

## Persistence Scenarios

1. Create a scene containing primitives, an imported model, texture, both light types, and a non-default camera.
2. Save the scene and confirm its metadata and copied resource directory are present.
3. Move the complete scene directory to another location and reopen it. Verify resources resolve through relative paths.
4. Attempt to open a malformed scene and a scene with a missing resource. Verify each load is rejected and the currently open scene remains intact.

## Agent Scenarios

1. Configure the selected Agent provider and submit: "Create a snowy night scene with three trees and a warm point-light street lamp; face the camera toward the lamp."
2. Verify each operation conforms to [the operation schema](contracts/scene-operations.schema.json), appears in the operation feedback, and results in the expected OpenGL scene.
3. Request a targeted material or placement change. Verify only the intended object changes.
4. Ask to modify a nonexistent object. Verify the Agent checks scene state, reports the issue, and does not modify an unrelated object.
5. Disable or remove Agent configuration. Verify direct scene editing, rendering, and save/load still work.

## Expected Outcomes

- The core graphics workflows in the spec are independently usable without Agent access.
- Saved scene directories can be moved as a unit and reopened with assets intact.
- Invalid operations or loads report errors without partial or unrelated scene changes.
- Baseline performance and Agent request success are recorded against the measurable criteria in `spec.md`.
