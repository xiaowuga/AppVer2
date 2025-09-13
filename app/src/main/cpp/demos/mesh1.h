#pragma once
#include <vector>
#include <string>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "shader.h"

#define MAX_BONE_INFLUENCE 4

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
    int BoneIDs[MAX_BONE_INFLUENCE];
    float Weights[MAX_BONE_INFLUENCE];
    Vertex() { 
        for (int i = 0 ; i < MAX_BONE_INFLUENCE; i++) {
            BoneIDs[i] = -1;
            Weights[i] = 0;
        }
    };
};

struct Texture {
    uint32_t id;
    std::string type;
    std::string path;
    bool active;
};

class Mesh {
public:
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
    void draw(Shader& shader);
    bool activeTexture(const std::string &textureName);
//private:
public:
    void setupMesh();
//private:
public:
    std::vector<Vertex>       mVertices;
    std::vector<unsigned int> mIndices;
    std::vector<Texture>      mTextures;
    unsigned int mFramebuffer;
    unsigned int mVAO;
    unsigned int mVBO;
    unsigned int mEBO;
};