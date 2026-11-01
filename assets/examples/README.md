# Example assets

Add only models and textures whose source and license can be documented.

`course-demo/` contains an original CC0 OBJ/MTL/PNG fixture, a portable saved scene, and the packaged resource copies. Open `course-demo/course-demo.json` from the editor's scene-file dialog. To regenerate the saved scene after editing the source fixture, build and run the `generate_course_demo` CMake target.

`../prefabs/` contains original snow-pine, street-lamp, and snow-cabin OBJ/MTL assets plus a snow-grain texture. Regenerate them with `python tools/generate_prefab_assets.py`. The course demo references their packaged copies.
