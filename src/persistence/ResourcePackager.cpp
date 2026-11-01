#include "persistence/ResourcePackager.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace scene_editor {
namespace {
namespace fs = std::filesystem;

std::string addResource(SceneDocument& document, const std::string& directory,
                        const fs::path& relative, SceneResource::Kind kind) {
    const std::string reference = (fs::path(directory) / relative).generic_string();
    document.resources.push_back({reference, kind});
    return reference;
}

fs::path sourceFile(const fs::path& path) {
    std::error_code error;
    const auto absolute = fs::canonical(path, error);
    if (error || !fs::is_regular_file(absolute))
        throw std::runtime_error("Missing source resource: " + path.string());
    return absolute;
}

std::string copyTexture(SceneDocument& document, const fs::path& source,
                        const fs::path& staging, const std::string& directory,
                        const fs::path& folder, unsigned& counter) {
    const auto input = sourceFile(source);
    const fs::path relative = folder / ("texture-" + std::to_string(counter++) + input.extension().string());
    fs::create_directories((staging / relative).parent_path());
    fs::copy_file(input, staging / relative);
    return addResource(document, directory, relative, SceneResource::Kind::Texture);
}

void packageMtl(SceneDocument& document, const fs::path& source,
                const fs::path& destination, const fs::path& folder,
                const fs::path& staging, const std::string& directory,
                unsigned& textureCounter) {
    std::ifstream input(source);
    std::ofstream output(destination);
    if (!input || !output) throw std::runtime_error("Cannot read or write MTL: " + source.string());
    std::string line;
    while (std::getline(input, line)) {
        std::istringstream tokens(line);
        std::string key;
        tokens >> key;
        if (key == "map_Kd") {
            std::string name, extra;
            tokens >> name >> extra;
            if (name.empty() || name[0] == '-' || !extra.empty())
                throw std::runtime_error("Unsupported MTL map_Kd syntax: " + source.string());
            const auto reference = copyTexture(document, source.parent_path() / name,
                                               staging, directory, folder, textureCounter);
            output << "map_Kd " << fs::path(reference).filename().generic_string() << '\n';
        } else if (key.rfind("map_", 0) == 0 || key == "bump" || key == "disp") {
            throw std::runtime_error("Unsupported MTL texture directive: " + key);
        } else {
            output << line << '\n';
        }
    }
    if (!output) throw std::runtime_error("Failed to write MTL: " + destination.string());
}

void packageObj(SceneDocument& document, SceneObject& object, unsigned modelIndex,
                const fs::path& staging, const std::string& directory,
                unsigned& textureCounter) {
    const fs::path source = sourceFile(object.geometryPath);
    if (source.extension() != ".obj")
        throw std::runtime_error("Only OBJ models can be packaged: " + source.string());
    const fs::path folder = "model-" + std::to_string(modelIndex);
    fs::create_directories(staging / folder);
    const fs::path destination = staging / folder / "model.obj";
    std::ifstream input(source);
    std::ofstream output(destination);
    if (!input || !output) throw std::runtime_error("Cannot read or write OBJ: " + source.string());
    std::string line;
    unsigned mtlIndex = 0;
    while (std::getline(input, line)) {
        std::istringstream tokens(line);
        std::string key;
        tokens >> key;
        if (key != "mtllib") {
            output << line << '\n';
            continue;
        }
        output << "mtllib";
        std::string name;
        bool found = false;
        while (tokens >> name) {
            found = true;
            const fs::path mtlSource = sourceFile(source.parent_path() / name);
            const fs::path mtlRelative = folder / ("material-" + std::to_string(mtlIndex++) + ".mtl");
            packageMtl(document, mtlSource, staging / mtlRelative, folder,
                       staging, directory, textureCounter);
            addResource(document, directory, mtlRelative, SceneResource::Kind::MaterialLibrary);
            output << ' ' << mtlRelative.filename().generic_string();
        }
        if (!found) throw std::runtime_error("OBJ has an empty mtllib line: " + source.string());
        output << '\n';
    }
    if (!output) throw std::runtime_error("Failed to write OBJ: " + destination.string());
    object.geometryPath = addResource(document, directory, folder / "model.obj", SceneResource::Kind::Model);
}
}

Result packageSceneResources(SceneDocument& document, const std::filesystem::path& stagingDirectory,
                             const std::string& finalDirectoryName) {
    try {
        fs::create_directories(stagingDirectory);
        document.resources.clear();
        unsigned modelIndex = 0, textureCounter = 0;
        for (auto& object : document.scene.objects) {
            if (object.kind == ObjectKind::ImportedModel)
                packageObj(document, object, modelIndex++, stagingDirectory,
                           finalDirectoryName, textureCounter);
            if (!object.material.texturePath.empty()) {
                object.material.texturePath = copyTexture(document, object.material.texturePath,
                    stagingDirectory, finalDirectoryName, "textures", textureCounter);
            }
        }
        return {};
    } catch (const std::exception& error) {
        return Result::failure(std::string("Could not package scene resources: ") + error.what());
    }
}
}
