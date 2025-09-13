//
// Created by 13055 on 2025/7/8.
//

#ifndef ROKIDOPENXRANDROIDDEMO_CADMESH_H
#define ROKIDOPENXRANDROIDDEMO_CADMESH_H

#include <vector>
#include <string>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "third_party/glm/gtc/type_ptr.hpp"
#include "demos/shader.h"
//#include "../pbrModel.h">

#define MAX_BONE_INFLUENCE 4

struct VertexFb {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
//    glm::vec3 Tangent;
//    glm::vec3 Bitangent;
    int BoneIDs[MAX_BONE_INFLUENCE];
    float Weights[MAX_BONE_INFLUENCE];
    VertexFb() {
        for (int i = 0 ; i < MAX_BONE_INFLUENCE; i++) {
            BoneIDs[i] = -1;
            Weights[i] = 0;
        }
    };
};

struct TextureFb {
    uint32_t id;
    std::string type;
    std::string path;
    bool active;
};

class CADMesh {
public:
    CADMesh(std::vector<VertexFb> vertices, std::vector<unsigned int> indices, std::vector<pbrMaterial> pbrMaterial);
    void draw(Shader& shader);
    bool activeTexture(const std::string &textureName);
private:
    void setupMesh();
private:
    std::vector<VertexFb>     mVertices;
    std::vector<unsigned int> mIndices;
    std::vector<pbrMaterial>  mPbrMaterial;
    std::vector<TextureFb>      mTextures;
    unsigned int mFramebuffer;
    unsigned int mVAO;
    unsigned int mVBO;
    unsigned int mEBO;
};
#endif //ROKIDOPENXRANDROIDDEMO_CADMESH_H
