---
description: "Dependency-ordered tasks for the OpenGL scene editor feature"
---

# Tasks: OpenGL 3D Scene Editor with AI Agent

**Input**: Design documents from `/specs/001-agent-3d-scene/`

**Prerequisites**: `plan.md`, `spec.md`, `research.md`, `data-model.md`, `contracts/`, `quickstart.md`

**Tests**: No separate automated test tasks are included because the specification does not explicitly request a test-first workflow. Each story retains its independent acceptance check; add tests where implementation policy or the course rubric requires them.

**Organization**: Tasks are grouped by user story. Shared application and scene foundations block all story phases.

## Phase 1: Setup

**Purpose**: Establish a reproducible native application build and source layout.

- [X] T001 Create the CMake project and target layout in `CMakeLists.txt` and `cmake/Dependencies.cmake`
- [X] T002 [P] Create application, renderer, scene, asset, persistence, agent, and UI source directories under `src/` per `plan.md`
- [X] T003 [P] Create shader and example asset directories under `assets/shaders/` and `assets/examples/`
- [X] T004 Document compiler, dependency setup, build, and launch steps in `README.md`
- [X] T005 Record dependency versions and license/provenance notes in `DEPENDENCIES.md`

---

## Phase 2: Foundational

**Purpose**: Build the window/render loop and shared scene model required by all stories.

- [X] T006 Initialize GLFW window, OpenGL 3.3 Core context, GLAD loading, and orderly shutdown in `src/app/Application.cpp`
- [X] T007 Implement shared vector, matrix, transform, and coordinate conventions in `src/scene/Transform.hpp` and `src/scene/Transform.cpp`
- [X] T008 Define stable scene object, material, light, camera, and resource types with validation in `src/scene/Scene.hpp` and `src/scene/Scene.cpp`
- [X] T009 Implement shader compilation, diagnostics, and GPU resource lifetime helpers in `src/renderer/ShaderProgram.cpp` and `src/renderer/GpuResource.hpp`
- [X] T010 Implement the render loop, viewport resize handling, depth testing, and frame timing in `src/renderer/Renderer.cpp`
- [X] T011 Add editor shell and viewport integration in `src/ui/EditorApp.cpp`
- [X] T012 Define centralized error/result types and user-readable error reporting in `src/app/Result.hpp` and `src/ui/Notifications.cpp`

**Checkpoint**: The application opens a valid OpenGL window, clears and presents frames, and owns a validated empty scene.

---

## Phase 3: User Story 1 - Build and View a Lit 3D Scene (Priority: P1) 🎯 MVP

**Goal**: Deliver the core computer graphics coursework experience: real-time rendering, geometry, camera navigation, materials, OBJ/MTL import, and multiple lights.

**Independent Test**: Add cube, sphere, and plane; import a supported OBJ with normals, UVs, MTL, and texture; navigate with the free camera; change a material and both light types; verify the visible result. Check the 100-object performance target on documented hardware.

### Implementation

- [X] T013 [P] [US1] Implement indexed cube, sphere, and plane mesh generation in `src/renderer/PrimitiveMeshes.cpp`
- [X] T014 [P] [US1] Implement vertex/index buffer and mesh upload/draw ownership in `src/renderer/Mesh.cpp` and `src/renderer/Mesh.hpp`
- [X] T015 [US1] Implement perspective camera matrices, yaw/pitch orientation, and projection validation in `src/scene/Camera.cpp`
- [X] T016 [US1] Implement keyboard/mouse free-camera movement and viewport focus handling in `src/ui/CameraControls.cpp`
- [X] T017 [US1] Implement Phong or Blinn-Phong vertex/fragment shaders with ambient, diffuse, and specular terms in `assets/shaders/phong.vert` and `assets/shaders/phong.frag`
- [X] T018 [US1] Upload scene transforms, material values, and point/directional light uniforms in `src/renderer/Lighting.cpp`
- [X] T019 [US1] Implement texture loading, GPU upload, and material texture binding in `src/renderer/Texture.cpp` and `src/renderer/MaterialBinding.cpp`
- [X] T020 [US1] Implement OBJ/MTL import for geometry, normals, texture coordinates, referenced materials, and textures in `src/assets/ObjImporter.cpp`
- [X] T021 [US1] Add imported mesh resources to the scene with stable IDs and actionable import errors in `src/assets/AssetService.cpp`
- [X] T022 [US1] Add primitive creation, object selection, and transform controls in `src/ui/SceneObjectPanel.cpp`
- [X] T023 [US1] Add material and texture editing controls in `src/ui/MaterialPanel.cpp`
- [X] T024 [US1] Add point and directional light controls for color/intensity and type-specific position/direction in `src/ui/LightPanel.cpp`
- [X] T025 [US1] Integrate scene traversal, visibility, mesh/material binding, and draw calls in `src/renderer/SceneRenderer.cpp`
- [X] T026 [US1] Document the supported OBJ/MTL subset and manual visual acceptance procedure in `docs/graphics-acceptance.md`

**Checkpoint**: Story 1 renders and edits primitives and imported models with a free camera, materials, textures, and both specified light types without Agent access.

---

## Phase 4: User Story 2 - Edit and Reopen a Scene (Priority: P2)

**Goal**: Support reliable object lifecycle and portable scene save/load while preserving the active scene on invalid loads.

**Independent Test**: Create a scene with primitives and imported resources, transforms, materials, lights, and a non-default camera; save, move the full scene directory, reopen, and compare restored state. Attempt malformed and missing-resource loads and verify the open scene remains intact.

### Implementation

- [X] T027 [P] [US2] Define the versioned scene JSON document and field mappings in `src/persistence/SceneDocument.hpp`
- [X] T028 [US2] Implement serialization/deserialization of object IDs, transforms, materials, lights, camera, and resource references in `src/persistence/SceneSerializer.cpp`
- [X] T029 [US2] Implement full document and relative resource validation before scene replacement in `src/persistence/SceneValidator.cpp`
- [X] T030 [US2] Copy imported OBJ, MTL, and texture dependencies into a sibling resource directory and rewrite relative references in `src/persistence/ResourcePackager.cpp`
- [X] T031 [US2] Implement save and load commands with failure-safe scene replacement in `src/persistence/SceneFileService.cpp`
- [X] T032 [US2] Add save/load controls and clear success/failure status in `src/ui/SceneFilePanel.cpp`
- [X] T033 [US2] Document portable scene directory layout, version behavior, and manual persistence acceptance in `docs/scene-format.md`

**Checkpoint**: Valid scenes reopen with all state and assets after moving the scene directory; invalid files leave the current scene unchanged.

---

## Phase 5: User Story 3 - Create and Refine a Scene with Natural Language (Priority: P3)

**Goal**: Layer an Agent over the scene operation contract to plan, execute, inspect, and report edits without making graphics depend on Agent availability.

**Independent Test**: Submit a multi-part scene brief, inspect structured operation results and rendered output, request a targeted edit, then reference a nonexistent object and verify scene lookup/error recovery. Disable the provider and confirm direct editing still works.

### Implementation

- [ ] T034 [P] [US3] Implement JSON request/result envelope validation from `contracts/scene-operations.schema.json` in `src/agent/OperationCodec.cpp`
- [ ] T035 [US3] Implement allowlisted operation dispatch for object, material, light, camera, state, render, save, and load actions in `src/agent/SceneOperationDispatcher.cpp`
- [ ] T036 [US3] Implement compact scene-state queries and stable-ID/name resolution in `src/agent/SceneStateQuery.cpp`
- [ ] T037 [P] [US3] Define provider adapter interface and configuration boundary in `src/agent/AgentProvider.hpp`
- [ ] T038 [US3] Implement selected provider request/response integration and bounded input/error handling in `src/agent/AgentProvider.cpp`
- [ ] T039 [US3] Implement multi-step plan execution with per-operation feedback and stop-on-unresolved-target behavior in `src/agent/PlanExecutor.cpp`
- [ ] T040 [US3] Keep clearable conversation context in the active editing session and exclude it from scene persistence in `src/agent/ConversationContext.cpp`
- [ ] T041 [US3] Add Agent chat, operation progress, completion summary, and clear-context controls in `src/ui/AgentPanel.cpp`
- [ ] T042 [US3] Document provider setup, structured-operation examples, failure behavior, and manual Agent acceptance in `docs/agent-setup.md`

**Checkpoint**: Supported natural-language requests execute as validated scene operations, return inspectable results, recover from missing references safely, and remain optional to the editor.

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Verify the course baseline and prepare a reproducible demonstration.

- [ ] T043 [P] Add a representative example scene and licensed OBJ/MTL/texture fixture in `assets/examples/course-demo/`
- [ ] T044 Document supported hardware, build instructions, scene limitations, dependency licenses, and demo workflow in `README.md`
- [ ] T045 Profile the 100-object scene for 60 seconds and record median frame rate, resolution, GPU, and driver in `docs/performance-results.md`
- [ ] T046 Run all scenarios in `specs/001-agent-3d-scene/quickstart.md` and record outcomes in `docs/acceptance-results.md`
- [ ] T047 Select at most one or two optional extensions only after baseline acceptance, and record the choice in `docs/optional-scope.md`

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies; completes the build layout and dependency setup.
- **Foundational (Phase 2)**: Depends on Setup; blocks all user story implementation.
- **User Story 1 (Phase 3)**: Depends on Foundation; delivers the OpenGL course MVP.
- **User Story 2 (Phase 4)**: Depends on Foundation and US1 scene/resource types; can begin after core scene types stabilize.
- **User Story 3 (Phase 5)**: Depends on Foundation and the scene operation surface from US1; can proceed after core scene mutations are available. Persistence tools integrate after US2.
- **Polish (Phase 6)**: Depends on the baseline stories selected for delivery; optional extensions are last.

### User Story Dependencies

- **US1 (P1)**: Starts after Phase 2 and is the independent MVP.
- **US2 (P2)**: Requires the scene entities and asset handling established in US1; it does not require Agent functionality.
- **US3 (P3)**: Requires scene operation dispatch over US1 capabilities. Save/load operations are connected after US2; core natural-language editing can be completed without them.

### Parallel Opportunities

- Setup tasks T002 and T003 can proceed in parallel with project configuration after the target directory conventions are agreed.
- In US1, primitive mesh generation (T013), camera (T015), and shader implementation (T017) touch separate files and can proceed in parallel after foundation interfaces stabilize. OBJ import (T020) can proceed alongside UI panels once scene types exist.
- In US2, document mapping (T027), resource packaging (T030), and UI panel work (T032) can proceed in parallel after serialization interfaces are agreed.
- In US3, codec work (T034), provider interface (T037), and chat panel layout (T041) can proceed in parallel after the operation contract is stable.
- Story phases are not fully independent: US2 and US3 reuse the scene model and operation surfaces, so integrate them sequentially or behind stable interfaces rather than assuming three isolated teams.

## Implementation Strategy

### MVP First

1. Complete Setup and Foundation.
2. Complete US1 and demonstrate a lit, navigable scene with primitives, OBJ/MTL assets, materials, textures, and multiple lights.
3. Validate the US1 acceptance scenario and performance target before expanding scope.

### Incremental Delivery

1. Add US2 persistence and portable resources to the stable graphics editor.
2. Add US3 Agent operations after direct editing and save/load are dependable.
3. Add no more than one or two optional extensions after baseline acceptance.

## Notes

- Every task follows the required checklist format with a sequential ID and concrete file path; story tasks carry their `[US#]` label.
- No test-only tasks are included because the spec does not explicitly request them. Manual acceptance scenarios remain tied to each story.
- Optional extensions are intentionally not broken into implementation tasks until one or two are selected after baseline completion.
