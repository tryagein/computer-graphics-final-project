# Scene file format (version 1)

The editor saves a UTF-8 JSON file and a sibling resource directory. For a file named `my-scene.json`, the directory is `my-scene.resources/`:

```text
saved-scene/
├── my-scene.json
└── my-scene.resources/
    ├── model-0/
    │   ├── model.obj
    │   ├── material-0.mtl
    │   └── texture-0.png
    └── textures/
        └── texture-1.png
```

Move or copy the JSON file and its matching `.resources` directory together. Paths stored in JSON are relative to the JSON file's parent directory. Model and texture paths are resolved to absolute runtime paths only after the whole document passes validation.

## Document fields

- `schema_version`: integer `1`. Other versions are rejected before the open scene changes.
- `objects`: stable `id`, `display_name`, `kind` (`cube`, `sphere`, `plane`, `imported_model`), `visible`, `geometry_ref`, `material_override`, `transform` (`position`, `rotation_degrees`, `scale`), and inline `material` (`base_color`, `ambient`, `diffuse`, `specular`, `emission`, `emission_strength`, `shininess`, `texture_ref`). Older version-1 scenes without the two emission fields load with zero emission.
- `lights`: stable `id`, `type` (`point` or `directional`), `position`, `direction`, `color`, `intensity`, and `enabled`.
- `camera`: `position`, `yaw`, `pitch`, `field_of_view`, `near_plane`, and `far_plane`.
- `resources`: every copied file's `relative_path` and `kind` (`model`, `material_library`, or `texture`).
- `selected_id`: optional active object selection. Agent conversation state is never included.

Vectors have three finite numeric components. Material and light colors are normalized to `[0, 1]`; scale components must be nonzero; light intensity is nonnegative. Object and light IDs are unique. Referenced files must exist inside the saved scene directory, be declared in `resources`, and use safe relative paths. The loader checks OBJ `mtllib` and MTL `map_Kd` dependencies before replacing the current scene. A malformed document, unsupported version, invalid value, missing file, or unsafe path leaves the active scene and camera unchanged.

Saving stages the JSON and assets before replacing an existing save. On an I/O failure, the previous save is restored where possible. Use the top toolbar's **场景文件** button to select a destination folder and enter a scene name. The editor creates `<name>.json` and `<name>.resources/` in that folder. Opening uses the system file picker. Errors are shown in the dialog and status bar.

## Packaged OBJ/MTL subset

OBJ `mtllib` entries with whitespace-separated filenames are copied and rewritten to local MTL files. MTL `map_Kd` accepts a single texture filename without options and is copied and rewritten beside the MTL. Texture directives such as bump, displacement, or option-bearing `map_Kd` lines are rejected on save with an error. These limits keep the portable package consistent with the renderer's diffuse-texture scope. Imported geometry and material editing remains available before saving.

MTL `Ke` is read as emissive color. It does not create a light entity: add a point light when the model should illuminate nearby geometry. Built-in prefabs package their source OBJ and MTL files through the same path as user-imported OBJ models.

## Manual acceptance

1. Add a cube and sphere, edit their transforms and materials, add both light types, move the camera, and import an OBJ with MTL and diffuse texture.
2. Choose **场景文件**, select a folder, enter `my-scene`, and save. Verify `my-scene.json` and `my-scene.resources/` appear in the selected folder.
3. Move the entire containing directory to another location. Open `my-scene.json` there and compare objects, IDs, transforms, materials, lights, camera view, and imported model appearance.
4. While the valid scene is open, try opening malformed JSON, then a copy with a missing resource. Confirm both operations report an error and leave the current view and scene intact.

Automated persistence tests exercise the round trip, moved directory, malformed JSON, unsupported version, unsafe path, and missing-resource rejection. Interactive visual comparison remains a manual check.
