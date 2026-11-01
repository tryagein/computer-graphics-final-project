cmake_minimum_required(VERSION 3.24)
project(OpenGLSceneEditor VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(SCENE_EDITOR_FETCH_DEPS "Fetch third-party dependencies with CMake" ON)
include(cmake/Dependencies.cmake)

add_executable(scene_editor
  src/main.cpp src/app/Application.cpp src/scene/Scene.cpp src/scene/Transform.cpp
  src/renderer/ShaderProgram.cpp src/renderer/Renderer.cpp src/renderer/Mesh.cpp
  src/renderer/PrimitiveMeshes.cpp src/renderer/Lighting.cpp src/scene/Camera.cpp
  src/renderer/Texture.cpp src/renderer/MaterialBinding.cpp src/renderer/SceneRenderer.cpp
  src/assets/ObjImporter.cpp src/assets/AssetService.cpp
  src/ui/EditorApp.cpp src/ui/CameraControls.cpp src/ui/SceneObjectPanel.cpp
  src/ui/MaterialPanel.cpp src/ui/LightPanel.cpp src/ui/Notifications.cpp)
target_include_directories(scene_editor PRIVATE src)
target_include_directories(scene_editor PRIVATE ${stb_SOURCE_DIR})
target_link_libraries(scene_editor PRIVATE glfw glad glm::glm assimp imgui nlohmann_json::nlohmann_json)
target_compile_definitions(scene_editor PRIVATE SCENE_EDITOR_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets")
if(MSVC)
  target_compile_options(scene_editor PRIVATE /W4 /permissive- /utf-8)
else()
  target_compile_options(scene_editor PRIVATE -Wall -Wextra -Wpedantic)
endif()
