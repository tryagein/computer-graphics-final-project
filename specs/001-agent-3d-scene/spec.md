# Feature Specification: OpenGL 3D Scene Editor with AI Agent

**Feature Branch**: `001-agent-3d-scene`

**Created**: 2026-11-01

**Status**: Draft

**Input**: User description: "基于 OpenGL 的交互式 3D 场景编辑课程项目，以实时渲染、基础几何体与 OBJ 模型、自由摄像机、Phong/Blinn-Phong 光照、纹理材质、多光源、对象增删改及场景保存加载为基础版本。AI Agent 提供自然语言场景编辑入口，通过结构化 JSON 工具调用进行多步规划、执行反馈、对话上下文和错误恢复。时间充足时再从阴影映射、天空盒、粒子、截图视觉检查、自动摄像机修正、Undo/Redo、操作历史或多 Agent 中选择一到两个扩展。"

## Clarifications

### Session 2026-11-01

- Q: 哪些光源类型必须支持基础版本？ → A: 点光源与方向光。
- Q: 导入 OBJ 模型时，基础版本需要保留哪些模型数据？ → A: 几何、法线、纹理坐标及 MTL 材质引用。
- Q: 保存场景时，导入的模型和纹理资源应如何处理？ → A: 将资源复制到场景旁的资源目录，并使用相对路径引用。
- Recommendation applied: Scene files use a documented, versioned format; loading validates the complete file before replacing the current scene.
- Recommendation applied: Each scene object has a stable unique identifier for selection, Agent references, and persistence.
- Recommendation applied: Conversation context is scoped to the current editing session; users can clear it, and scene files do not store conversation history.
- Recommendation applied: If the Agent service is unavailable or a request exceeds its supported input size, direct graphics editing remains available and the user receives a clear status message.
- Recommendation applied: Initial evaluation is single-user and single-session; concurrent editing, authentication, and cloud synchronization are out of scope.

## User Scenarios & Testing *(mandatory)*

### User Story 1 - Build and View a Lit 3D Scene (Priority: P1)

As a student, I create a scene from basic shapes or imported models, move around it with a free camera, and see its materials and lights update in a real-time rendered view.

**Why this priority**: The graphics application is the core course deliverable. Scene editing and rendering must provide value even when the Agent is unavailable.

**Independent Test**: Create a scene containing a plane, cube, sphere, and imported OBJ model; move the camera; apply a texture and material; add multiple lights; confirm the visible result changes as expected.

**Acceptance Scenarios**:

1. **Given** an empty scene, **When** the user adds a cube, sphere, and plane and moves the camera, **Then** all three objects are visible from the selected viewpoint and can be distinguished in the rendered view.
2. **Given** a supported OBJ file, **When** the user imports it, **Then** the model appears in the scene and can be positioned and scaled.
3. **Given** an object with a material and multiple active lights, **When** the user changes its material or a light's properties, **Then** the real-time view updates to reflect the change.

---

### User Story 2 - Edit and Reopen a Scene (Priority: P2)

As a student, I add, select, transform, and delete scene objects, then save the scene and reopen it with its contents intact.

**Why this priority**: Reliable editing and persistence make the graphics work usable as a complete coursework application rather than a one-time rendering demo.

**Independent Test**: Create a scene with several objects, distinct materials, lights, and a camera position; save, reopen, and compare the scene contents and visible result.

**Acceptance Scenarios**:

1. **Given** a scene with several objects, **When** the user adds, moves, scales, or deletes a selected object, **Then** only the intended object changes and the rendered view reflects the edit.
2. **Given** a configured scene, **When** the user saves and then reopens it, **Then** objects, transforms, materials, lights, and camera settings are restored.
3. **Given** a file that is not a valid saved scene, **When** the user attempts to load it, **Then** the application reports the problem and keeps the current scene usable.

---

### User Story 3 - Create and Refine a Scene with Natural Language (Priority: P3)

As a user, I describe a scene or a targeted change in natural language and let the Agent perform supported scene operations while showing what it did.

**Why this priority**: The Agent makes scene authoring more accessible, while remaining an interaction layer over the independently useful graphics editor.

**Independent Test**: Request a scene with a known set of supported objects, materials, lights, and camera framing; compare the resulting scene and operation feedback with the request, then request a follow-up edit.

**Acceptance Scenarios**:

1. **Given** an empty scene, **When** the user requests a snowy night scene with three trees, a warm street lamp, and a camera facing the lamp, **Then** the Agent creates the supported scene content and the OpenGL view displays it.
2. **Given** a populated scene, **When** the user asks to change one object's material, **Then** the Agent identifies the intended object, performs a structured scene operation, and leaves unrelated content unchanged.
3. **Given** an operation referring to a missing object, **When** the operation fails, **Then** the Agent queries the current scene state, explains the mismatch, and retries only when the target can be identified safely.
4. **Given** a multi-part request, **When** the Agent plans and executes multiple operations, **Then** the user can see progress or results and receives a final summary of completed and unsupported parts.

### Edge Cases

- The user imports a missing, malformed, or unsupported OBJ file; the application reports the problem and keeps the current scene intact.
- The requested object type, material property, or light configuration is unsupported; the system identifies unsupported parts and continues with supported parts when possible.
- A requested edit refers to an object that does not exist or cannot be uniquely identified; the Agent checks scene state and asks a focused follow-up instead of changing an unrelated object.
- Structured tool output has missing, invalid, or out-of-range arguments; the operation is rejected with an actionable error and does not corrupt the scene.
- Saving or loading fails; the application reports the reason and preserves the current usable scene state.
- The Agent service is unavailable; users can still create, edit, render, save, and load scenes through the graphics application.
- Real-time rendering performance drops as scene complexity grows; the application continues to accept edits and indicates when the scene exceeds its supported workload.

## Requirements *(mandatory)*

### Functional Requirements

#### Core Graphics Requirements

- **FR-001**: The application MUST provide a real-time rendered 3D view of the current scene.
- **FR-002**: Users MUST be able to create and edit scenes containing cube, sphere, and plane primitives.
- **FR-003**: Users MUST be able to import OBJ models with geometry, normals, texture coordinates, and referenced MTL materials and textures, then place, scale, and remove them in the scene.
- **FR-004**: Users MUST be able to navigate the scene with a free camera and control its viewing direction.
- **FR-005**: The renderer MUST support Phong or Blinn-Phong lighting with ambient, diffuse, and specular contributions.
- **FR-006**: Users MUST be able to assign materials and textures to scene objects and adjust supported material properties, including color and roughness or shininess.
- **FR-007**: Users MUST be able to add and configure multiple point lights and directional lights, including color and intensity; point lights MUST also expose position, and directional lights MUST expose direction.
- **FR-008**: Every scene object MUST have a stable unique identifier, and users MUST be able to add, select, transform, and delete an identified object without unintentionally changing unrelated objects.
- **FR-009**: Users MUST be able to save a scene in a documented, versioned format and load it later with object identifiers, geometry references, transforms, materials, lights, and camera settings restored. Imported models and textures MUST be copied to a resource directory alongside the saved scene and referenced using relative paths.
- **FR-010**: The application MUST report invalid model or scene files without discarding the last valid in-memory scene.
- **FR-021**: The application MUST validate a complete saved scene and its resource references before replacing the current scene during load.

#### Agent Interaction Requirements

- **FR-011**: The system MUST accept natural-language requests for scene creation and edits, and translate them into supported scene operations.
- **FR-012**: Agent-requested scene changes MUST use validated structured JSON or equivalent structured tool calls; the Agent MUST NOT generate or execute OpenGL source code.
- **FR-013**: The Agent MUST be able to plan and execute a multi-step request using the available scene operations.
- **FR-014**: The scene system MUST return each operation's success or failure and relevant resulting state to the Agent.
- **FR-015**: The Agent MUST retain conversation context for follow-up requests within the current editing session, provide a way for the user to clear that context, and MUST NOT store conversation history in saved scene files.
- **FR-016**: When an operation fails because an object is absent or ambiguous, the Agent MUST query the current scene state and either resolve the reference safely or ask the user for clarification.
- **FR-017**: The system MUST report completed, failed, and unsupported parts of an Agent request in a readable summary.
- **FR-018**: Failure or unavailability of the Agent MUST NOT disable direct scene editing, real-time rendering, or scene persistence.
- **FR-022**: When the Agent service is unavailable or a request exceeds the supported input size, the system MUST report the limitation and keep direct graphics editing available.

#### Optional Extension Scope

- **FR-019**: After all core graphics and Agent requirements are complete, the project MAY implement one or two optional extensions selected from: shadow mapping, skybox, particle system, screenshot-based visual checking, automatic camera correction, Undo/Redo, scene operation history, or specialized planning/execution/checking agents.
- **FR-020**: Optional extensions MUST NOT replace or reduce the acceptance scope of core graphics requirements.

#### Scope Boundaries

- **FR-023**: The first release MUST support one local user editing one active scene at a time; concurrent editing, authentication, and cloud synchronization are out of scope.

### Key Entities *(include if feature involves data)*

- **Scene**: Editable collection of objects, materials, lights, and camera settings, with a current rendered view.
- **Scene Object**: A uniquely identifiable primitive or imported model with geometry, transform, and material assignment.
- **Material and Texture**: Surface appearance assigned to an object, including color and supported lighting response.
- **Light**: Configurable illumination source with a type, position, color, and intensity.
- **Camera**: Free navigable viewpoint with position, orientation, and view direction.
- **Saved Scene**: Persistent representation of scene contents and settings that can be reopened.
- **Scene Resource**: An imported model or texture copied beside the saved scene and referenced by a relative path.
- **Scene Operation**: A validated structured request to create, inspect, update, delete, save, load, or render scene state, together with its result.
- **Conversation Context**: Recent user instructions, Agent plans, tool outcomes, and scene references used to interpret follow-up requests.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: A user can create a scene with all three primitive types, navigate it with the camera, and observe the result in the rendered view in 9 out of 10 guided evaluation attempts.
- **SC-002**: In a set of 10 representative OBJ files declared supported by the project, at least 9 load and can be transformed in the scene.
- **SC-003**: In 9 out of 10 lighting and material evaluation cases, changing an object's material or one of multiple lights produces the expected visible change while leaving unrelated object transforms unchanged.
- **SC-004**: In 10 save-and-reopen trials, all saved objects, transforms, materials, lights, and camera settings are restored without data loss.
- **SC-005**: On the documented target machine, the base test scene of up to 100 objects renders at a median of at least 30 frames per second during a 60-second camera-navigation run.
- **SC-006**: For 20 supported natural-language scene requests, the Agent correctly applies at least 90% of explicitly stated object, material, lighting, and camera requirements.
- **SC-007**: For 10 missing-object or invalid-operation cases, the Agent reports the failure, checks current scene state when relevant, and does not silently alter an unrelated object in all 10 cases.
- **SC-008**: Users can complete the core scene creation, editing, rendering, and save/load workflow without invoking the Agent.
- **SC-009**: After saving a scene with imported models and textures, moving the complete scene directory to another location preserves all resource links in 10 out of 10 reopen trials.
- **SC-010**: In 10 invalid-scene load trials, the application rejects the file and preserves the previously open scene in all 10 cases.

## Assumptions

- OpenGL real-time scene rendering and scene editing are the primary course deliverables; the Agent is a secondary interaction layer over those capabilities.
- The baseline scope includes the core graphics requirements and Agent interaction requirements in this specification; optional extensions are not required for baseline completion.
- The project targets a single local user and one active scene; collaboration and cloud scene storage are out of scope.
- OBJ support is limited to geometry, normals, texture coordinates, and referenced MTL materials and textures; unsupported OBJ features are reported to the user.
- The specific Agent provider and whether structured calls use a hosted service or local model are planning decisions; the graphics editor remains usable if that service is unavailable.
- Scene files use a documented, versioned format, and loading validates the complete scene before replacing the active scene.
- Conversation context exists only for the active editing session, can be cleared by the user, and is not persisted in scene files.
- Evaluation covers one local user and one active session; concurrent editing, authentication, and cloud synchronization are out of scope.
- One or two optional extensions may be selected only after the base scope is functional and demonstrated.
- Performance measurements use a target machine and representative scene size documented during planning.

## Phase 7 extension after baseline acceptance

The graphics-focused extension adds three bundled, multi-material OBJ prefabs (`snow_pine`, `street_lamp`, `snow_cabin`) and a snow texture. The toolbar and Agent `add_prefab` tool use fixed asset IDs; the street lamp also creates a warm point light. The renderer reads MTL emissive color and adds one directional-light depth-map pass with adjustable bias and PCF radius. Saved scenes package prefab OBJ/MTL/PNG dependencies through the existing portable resource system. Implementation limits and evidence are in `docs/phase7-graphics.md` and `docs/phase7-acceptance.md`.
