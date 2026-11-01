# Graphics acceptance: T001–T026

## Supported OBJ and MTL subset

- Import OBJ files with triangle or polygon faces, positions, normals, and first UV channel. Polygons are triangulated; absent normals are generated.
- Referenced MTL diffuse color, specular color, shininess, and first diffuse texture are read. More advanced material maps, animation, embedded textures, and non-OBJ formats are outside this baseline.
- Use paths to local OBJ files. Relative texture paths are resolved against the OBJ directory. Import errors do not add an incomplete object.
- Rendering supports at most four point lights and two directional lights in the current shader; extra lights remain in scene state but are not rendered.

## Manual visual acceptance

1. Launch the editor on a machine supporting OpenGL 3.3 Core. A plane and cube should appear under a directional and a warm point light.
2. Add a cube, sphere, and plane from the Objects panel. Select each and change position, rotation, and scale. Only the selected object's transform should change.
3. Hold the right mouse button over the viewport to look around; use W/A/S/D to move relative to the camera and Q/E to move vertically. The view should update on each frame.
4. Change base color, ambient, diffuse, specular, and shininess in Material. The selected surface should change visibly. Enter a valid texture path and verify the image is mapped using UV coordinates.
5. Import `assets/examples/pyramid.obj` to verify geometry, generated normals, and the referenced MTL material. Then import an OBJ with UVs and a referenced diffuse texture to verify texture mapping and transform the imported object.
6. Add a point light and directional light. Change color, intensity, position or direction. Verify distinct visible effects; disable each to verify its contribution disappears.
7. Inspect the Status panel for frame render time and watch for import/shader errors. Record viewport size, GPU, and driver before comparing performance with SC-005.

This document is an acceptance procedure, not a claim that a human visual check has been completed.
