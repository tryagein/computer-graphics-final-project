# Implementation Plan: OpenGL 3D Scene Editor with AI Agent

**Branch**: `001-agent-3d-scene` | **Date**: 2026-11-01 | **Spec**: [spec.md](spec.md)

**Input**: Feature specification from `/specs/001-agent-3d-scene/spec.md`

## Summary

Deliver a coursework-ready interactive 3D editor whose primary value is a real-time OpenGL renderer and dependable scene editing, import, and persistence. Build a clear scene model and graphics pipeline first; layer the natural-language Agent on a validated scene-operation contract so the editor remains useful when the Agent service is unavailable. Start with the specified primitives, OBJ/MTL support, free camera, Phong or Blinn-Phong shading, textures, point and directional lights, and portable save/load. Defer all optional extensions until the baseline is demonstrated.

## Technical Context

**Language/Version**: C++17 (planning assumption; confirm compiler/toolchain compatibility during implementation setup)

**Primary Dependencies**: OpenGL 3.3 Core profile; GLFW for window/context/input; GLAD for function loading; GLM for graphics mathematics; Assimp for OBJ/MTL import; Dear ImGui for editor controls; nlohmann/json for scene and tool-call serialization; an Agent provider adapter behind a configuration boundary.

**Storage**: Versioned JSON scene file plus sibling resource directory containing copied OBJ, MTL, and texture assets; paths inside the scene file are relative. Conversation context stays in memory for the active session.

**Testing**: CMake build; unit tests for scene validation, transforms, serialization, resource path handling, and operation dispatch; integration checks for model import and render setup; manual visual checks for camera, materials, lighting, and real-time behavior. Select and document a test framework during setup.

**Target Platform**: Windows desktop as the initial course target; keep windowing and build configuration portable where practical.

**Project Type**: Single native desktop application.

**Performance Goals**: Spec target: median >=30 FPS for a 100-object base scene during 60 seconds of camera navigation on documented hardware. Keep direct manipulation responsive while Agent requests are pending.

**Constraints**: OpenGL is the graphics coursework focus; agent operations must be structured and validated and cannot execute generated graphics code. OBJ scope covers geometry, normals, texture coordinates, and referenced MTL materials/textures. Load validation must complete before replacing the active scene. Imported resources must travel with the saved scene directory. Agent service provider, credentials, and network behavior must be isolated from the graphics core.

**Scale/Scope**: One local user, one active scene, up to 100 objects in the performance baseline, multiple point and directional lights, and one or two optional extensions only after baseline completion.

## Constitution Check

| Principle | Gate | Plan alignment |
|---|---|---|
| Visual Correctness and Mathematical Clarity | Pass | Define coordinate conventions, camera behavior, lighting terms, and numerical limits in the renderer and contracts. |
| Modular, Reproducible Implementation | Pass | Separate graphics, scene state, persistence/import, UI, and Agent adapter; document dependencies and build steps. |
| Verification of Visible Behavior | Pass | Pair deterministic tests with repeatable visual acceptance scenarios from the spec. |
| Performance Proportional to the Workload | Pass | Measure the specified 100-object scene and avoid blocking rendering on Agent calls. |
| Readability and Explainable Complexity | Pass | Keep core algorithms understandable; defer optional features until baseline is complete. |
| Asset provenance and compatible usage | Pass | Record asset source/license and package imported user assets beside the scene. |

No constitution gate violations identified. The exact Agent provider and test framework are setup decisions and must not compromise a provider-independent scene-operation contract.

## Project Structure

### Documentation (this feature)

```text
specs/001-agent-3d-scene/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
│   └── scene-operations.schema.json
└── tasks.md                 # created by $speckit-tasks
```

### Source Code (repository root)

```text
src/
├── app/                     # application lifecycle and editor shell
├── renderer/                # OpenGL renderer, shaders, mesh and texture resources
├── scene/                   # scene entities, transforms, materials, lights, camera
├── assets/                  # OBJ/MTL import and portable resource copying
├── persistence/             # versioned scene serialization and validation
├── agent/                   # provider adapter, planner interface, context, tool dispatch
└── ui/                      # viewport, object controls, lighting/material panels, chat
assets/
├── shaders/
└── examples/
tests/
├── unit/
├── integration/
└── manual/
```

**Structure Decision**: One C++ desktop application with focused internal modules. This avoids splitting a course-scale product into independently deployed services while preserving a strict boundary between Agent orchestration and scene execution.

## Complexity Tracking

No constitution violations requiring justification. The Agent adapter is a separate module because provider/network failure must not affect direct graphics editing; it is not a separately deployed service.
