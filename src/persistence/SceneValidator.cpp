#include "persistence/SceneValidator.hpp"
#include <algorithm>
#include <fstream>
#include <map>
#include <set>
#include <sstream>

namespace scene_editor {
namespace {
namespace fs = std::filesystem;

bool inside(const fs::path& root, const fs::path& path) {
    auto left = root.begin();
    auto right = path.begin();
    while (left != root.end()) {
        if (right == path.end() || *left != *right) return false;
        ++left;
        ++right;
    }
    return true;
}

Result resolveResource(const fs::path& root, const std::string& reference, fs::path& resolved) {
    const fs::path relative = fs::u8path(reference);
    if (reference.empty() || relative.is_absolute() || relative.has_root_name() ||
        std::find(relative.begin(), relative.end(), fs::path("..")) != relative.end())
        return Result::failure("Unsafe scene resource path: " + reference);
    std::error_code error;
    resolved = fs::canonical(root / relative, error);
    if (error || !inside(root, resolved) || !fs::is_regular_file(resolved))
        return Result::failure("Missing or invalid scene resource: " + reference);
    return {};
}

bool normalizedColor(const glm::vec3& color) {
    for (int i = 0; i < 3; ++i)
        if (color[i] < 0 || color[i] > 1) return false;
    return true;
}

Result checkDependency(const fs::path& root, const fs::path& base,
                       const std::string& reference, const std::map<fs::path, SceneResource::Kind>& declared,
                       SceneResource::Kind expected) {
    fs::path resolved;
    const fs::path relative = (base / fs::u8path(reference)).lexically_relative(root);
    auto result = resolveResource(root, relative.generic_u8string(), resolved);
    if (!result.ok) return result;
    const auto found = declared.find(resolved);
    if (found == declared.end() || found->second != expected)
        return Result::failure("Undeclared OBJ/MTL dependency: " + reference);
    return {};
}
}

Result validateSceneDocument(SceneDocument& document, const std::filesystem::path& sceneFile) {
    namespace fs = std::filesystem;
    if (document.schemaVersion != kSceneSchemaVersion)
        return Result::failure("Unsupported scene schema version: " + std::to_string(document.schemaVersion));
    std::error_code error;
    const fs::path root = fs::canonical(sceneFile.parent_path().empty() ? fs::path(".") : sceneFile.parent_path(), error);
    if (error) return Result::failure("Scene directory is not accessible.");
    std::set<std::string> ids, objectIds;
    std::map<fs::path, SceneResource::Kind> declared;
    for (const auto& resource : document.resources) {
        if (!resource.valid()) return Result::failure("Scene contains an empty resource reference.");
        fs::path path;
        auto result = resolveResource(root, resource.relativePath, path);
        if (!result.ok) return result;
        if (!declared.emplace(path, resource.kind).second)
            return Result::failure("Duplicate resource reference: " + resource.relativePath);
    }
    for (auto& object : document.scene.objects) {
        if (!object.valid() || !ids.insert(object.id).second)
            return Result::failure("Invalid or duplicate object ID: " + object.id);
        objectIds.insert(object.id);
        const auto& material = object.material;
        if (!normalizedColor(material.color) || !normalizedColor(material.ambient) ||
            !normalizedColor(material.diffuse) || !normalizedColor(material.specular) ||
            !normalizedColor(material.emission))
            return Result::failure("Material color is outside [0, 1]: " + object.id);
        if (object.kind == ObjectKind::ImportedModel) {
            fs::path path;
            auto result = resolveResource(root, object.geometryPath, path);
            if (!result.ok) return result;
            if (declared.find(path) == declared.end() ||
                declared.at(path) != SceneResource::Kind::Model || path.extension() != ".obj")
                return Result::failure("Imported object does not reference a declared OBJ: " + object.id);
            object.geometryPath = path.string();
        } else if (!object.geometryPath.empty()) {
            return Result::failure("Primitive object has a geometry reference: " + object.id);
        }
        if (!material.texturePath.empty()) {
            fs::path path;
            auto result = resolveResource(root, material.texturePath, path);
            if (!result.ok) return result;
            if (declared.find(path) == declared.end() || declared.at(path) != SceneResource::Kind::Texture)
                return Result::failure("Material texture is not declared: " + object.id);
            object.material.texturePath = path.string();
        }
    }
    for (const auto& light : document.scene.lights) {
        if (!light.valid() || !ids.insert(light.id).second || !normalizedColor(light.color))
            return Result::failure("Invalid or duplicate light: " + light.id);
    }
    if (!document.scene.selectedId.empty() && !objectIds.count(document.scene.selectedId))
        return Result::failure("Selected object no longer exists.");

    // The OBJ/MTL files are also checked so their own relative references cannot escape the package.
    for (const auto& resource : document.resources) {
        if (resource.kind != SceneResource::Kind::Model &&
            resource.kind != SceneResource::Kind::MaterialLibrary) continue;
        fs::path file;
        auto result = resolveResource(root, resource.relativePath, file);
        if (!result.ok) return result;
        std::ifstream input(file);
        if (!input) return Result::failure("Cannot read scene resource: " + resource.relativePath);
        std::string line;
        while (std::getline(input, line)) {
            std::istringstream tokens(line);
            std::string key;
            tokens >> key;
            if (resource.kind == SceneResource::Kind::Model && key == "mtllib") {
                std::string name;
                bool found = false;
                while (tokens >> name) {
                    found = true;
                    result = checkDependency(root, file.parent_path(), name, declared,
                                             SceneResource::Kind::MaterialLibrary);
                    if (!result.ok) return result;
                }
                if (!found) return Result::failure("OBJ has an empty mtllib line.");
            } else if (resource.kind == SceneResource::Kind::MaterialLibrary && key == "map_Kd") {
                std::string name;
                tokens >> name;
                if (name.empty() || name[0] == '-')
                    return Result::failure("Unsupported MTL map_Kd reference in " + resource.relativePath);
                result = checkDependency(root, file.parent_path(), name, declared,
                                         SceneResource::Kind::Texture);
                if (!result.ok) return result;
            }
        }
    }
    return {};
}
}
