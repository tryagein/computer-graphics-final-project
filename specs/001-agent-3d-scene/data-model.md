# Data Model: OpenGL 3D Scene Editor with AI Agent

## Scene

- `schema_version`: required positive integer identifying the persistence format.
- `objects`: zero or more `SceneObject` records, keyed by stable unique ID.
- `lights`: zero or more `Light` records, each with stable unique ID.
- `camera`: one `Camera` record.
- `materials`: reusable `Material` records or inline references, as selected during implementation.
- `resources`: relative references to imported geometry and texture files.

Validation: IDs are unique within the scene; all referenced materials and resources exist; numeric values are finite; required vectors have three components; unsupported schema versions fail before replacing the current scene.

## SceneObject

- `id`: stable unique identifier.
- `kind`: `cube`, `sphere`, `plane`, or `imported_model`.
- `transform`: translation, rotation, and scale.
- `geometry_ref`: required for imported models; points to a scene-relative resource.
- `material_ref`: material assigned to the object.
- `display_name`: optional user-facing label; not used as identity.

Validation: transform values are finite; scale components are nonzero; imported geometry references resolve. Agent operations address objects by `id`, using display names only to resolve user language.

## Material

- `id`: stable identifier.
- `base_color`: RGB or RGBA values in the documented normalized range.
- `ambient`, `diffuse`, `specular`: Phong/Blinn-Phong material terms.
- `shininess`: positive scalar for specular highlight size.
- `texture_refs`: optional scene-relative texture references.

Validation: components are finite and within documented ranges. The chosen shading model maps material values consistently.

## Light

- `id`: stable identifier.
- `type`: `point` or `directional`.
- `color`: RGB values in the documented range.
- `intensity`: finite nonnegative scalar.
- `position`: required for point lights.
- `direction`: required and nonzero for directional lights.
- `enabled`: boolean.

Validation: required type-specific fields exist; direction is normalized during validation or rejected if zero; the implementation documents any supported light-count limit.

## Camera

- `position`: world-space location.
- `orientation`: yaw/pitch or equivalent orientation representation.
- `target`: optional focus target used when framing a scene or persisting look-at state.
- `field_of_view`, `near_plane`, `far_plane`: projection parameters.

Validation: finite values; field of view strictly between 0 and 180 degrees; near plane positive and less than far plane.

## SceneResource

- `relative_path`: path from the saved scene directory to a copied asset.
- `kind`: model, material library, or texture.
- `source_name`: optional provenance metadata.

Lifecycle: import copies source files to the scene resource area; save writes relative references; load validates every reference before scene replacement. Missing or invalid references reject the load while retaining the current scene.

## SceneOperation

- `operation_id`: request-scoped identifier for correlating result feedback.
- `tool`: allowlisted operation name.
- `arguments`: operation-specific object IDs, values, or resource paths.
- `result`: success/failure, message, and relevant changed entity IDs or state summary.

Lifecycle: parse → validate name and arguments → apply to a valid scene state → report result. Invalid operations do not partially mutate state. The Agent may inspect the scene and retry only after resolving missing or ambiguous object references.

## ConversationContext

- Current-session messages, plans, tool calls, and tool results needed to interpret follow-up requests.
- Clearable by the user; kept out of saved scene documents.
- Not shared across users or persisted to cloud storage in the first release.
