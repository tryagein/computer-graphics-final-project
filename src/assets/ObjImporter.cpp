#include "assets/ObjImporter.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <filesystem>

namespace scene_editor {
ImportedAsset importObj(const std::string& path) {
    ImportedAsset result;
    const std::filesystem::path file(path);
    if (file.extension() != ".obj") {
        result.error = "Only OBJ files are supported";
        return result;
    }
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate |
        aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);
    if (!scene || !scene->HasMeshes()) {
        result.error = importer.GetErrorString();
        return result;
    }
    for (unsigned k = 0; k < scene->mNumMeshes; ++k) {
        const aiMesh* mesh = scene->mMeshes[k];
        ImportedSubmesh item;
        item.vertices.reserve(mesh->mNumVertices);
        for (unsigned i = 0; i < mesh->mNumVertices; ++i) {
            const auto& p = mesh->mVertices[i];
            const auto& n = mesh->mNormals[i];
            glm::vec2 uv{0};
            if (mesh->HasTextureCoords(0))
                uv = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
            item.vertices.push_back({{p.x, p.y, p.z}, {n.x, n.y, n.z}, uv});
        }
        for (unsigned i = 0; i < mesh->mNumFaces; ++i)
            for (unsigned j = 0; j < mesh->mFaces[i].mNumIndices; ++j)
                item.indices.push_back(mesh->mFaces[i].mIndices[j]);
        if (mesh->mMaterialIndex < scene->mNumMaterials) {
            auto* mat = scene->mMaterials[mesh->mMaterialIndex];
            aiColor3D color;
            if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
                item.material.color = {color.r, color.g, color.b};
            if (mat->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS)
                item.material.specular = {color.r, color.g, color.b};
            if (mat->Get(AI_MATKEY_COLOR_EMISSIVE, color) == AI_SUCCESS) {
                item.material.emission = {color.r, color.g, color.b};
                item.material.emissionStrength = 1.4F;
            }
            float shine;
            if (mat->Get(AI_MATKEY_SHININESS, shine) == AI_SUCCESS && shine > 0)
                item.material.shininess = shine;
            aiString texture;
            if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &texture) == AI_SUCCESS)
                item.material.texturePath = (file.parent_path() / texture.C_Str()).lexically_normal().string();
        }
        result.submeshes.push_back(std::move(item));
    }
    return result;
}
}
