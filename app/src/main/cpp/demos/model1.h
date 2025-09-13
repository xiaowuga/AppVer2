#pragma once
#include <string>
#include <map>
#include <vector>
#include <memory>
#include "mesh.h"
#include "shader.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "shader.h"


class Model {
public:
    Model() = delete;
    Model(const std::string& name, bool hasBoneInfo = false);
    ~Model();

    std::string& name();

    bool loadModel(const std::string& modelFileName);
    bool loadLocalModel(const std::string &modelFileName,const std::string &importer_type="");

    bool initialize() { return false; };

    bool bindMeshTexture(const std::string& meshName, const std::string& textureName);
    bool activeMeshTexture(const std::string& meshName, const std::string& textureName);

    bool render(const glm::mat4& p, const glm::mat4& v, const glm::mat4& m);

    int getBoneNodeIndexByName(const std::string& name) const;

    void setBoneNodeMatrices(const std::string& bone, const glm::mat4& m);
    //=========================================================================

    //@file_path: 必须是可直接访问的 Sdcard 路径
    static Model GetModelFromLocalFile(const std::string &model_name,const std::string &file_path);
    bool is_valid()const;
private:
    bool _valid{false};
    //=========================================================================

private:
    void initShader();
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
    std::vector<Texture> loadMaterialTextures_force(aiMaterial* mat, aiTextureType type, std::string typeName, std::string file);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    void processMeshBone(aiMesh* mesh, std::vector<Vertex>& vertices);
    void initializeBoneNode();
    void draw();

private:
    std::string mName;
    std::map<std::string, Mesh> mMeshes;
    bool mHasBoneInfo;
    
    struct boneInfo {
        int id;
        boneInfo(int count) : id(count) {};
    };
    std::map<std::string, std::shared_ptr<boneInfo>> mBoneInfoMap;

    bool mIsGammaCorrection;

    std::vector<Texture> mTexturesLoaded;
    std::string mDirectory;

    std::map<std::string, std::vector<std::string>> mMeshTexturesMap;

    static Shader mShader;
};
