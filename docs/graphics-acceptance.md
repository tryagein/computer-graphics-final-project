# Graphics acceptance: T001–T026

## Supported OBJ and MTL subset

- Import OBJ files with triangle or polygon faces, positions, normals, and first UV channel. Polygons are triangulated; absent normals are generated.
- Referenced MTL diffuse color, specular color, shininess, and first diffuse texture are read. More advanced material maps, animation, embedded textures, and non-OBJ formats are outside this baseline.
- Use paths to local OBJ files. Relative texture paths are resolved against the OBJ directory. Import errors do not add an incomplete object.
- Rendering supports at most four point lights and two directional lights in the current shader; extra lights remain in scene state but are not rendered.

## Manual visual acceptance

1. Launch the editor on a machine supporting OpenGL 3.3 Core. A plane and cube should appear under a directional and a warm point light.
2. Use the top toolbar to add a cube, sphere, and plane. Each should appear in the center viewport and become selected in the left Scene panel. Change position, rotation, and scale in the right Properties tab; only the selected object should change.
3. Click the viewport, hold the right mouse button to look around, use W/A/S/D to move, and Q/E to move vertically. Press Reset camera on the toolbar to return to the starting view.
4. Change base color, ambient, diffuse, specular, and shininess in Properties. The selected surface should change visibly. Enter a valid texture path and verify the image is mapped using UV coordinates.
5. Click Example model on the toolbar to verify OBJ geometry, generated normals, and the referenced MTL material. Use Import OBJ with an OBJ that has UVs and a diffuse texture to verify texture mapping.
6. Open the right Lighting tab. Add a point light and directional light. Change color, intensity, position or direction. Verify distinct visible effects; disable each to verify its contribution disappears.
7. Inspect the bottom status bar for frame render time and import errors. Record viewport size, GPU, and driver before comparing performance with SC-005.

This document is an acceptance procedure, not a claim that a human visual check has been completed.
