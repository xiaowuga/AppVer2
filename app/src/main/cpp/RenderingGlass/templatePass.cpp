//
// Created by gyp on 2025/7/6.
//
#include "RenderingGlass/templatePass.h"
#include "utils.h"

TemplatePass::~TemplatePass() = default;

void TemplatePass::initShader() {
    std::vector<char> vertexShaderCode = readFileFromAssets("shaders/template.vert");
    std::vector<char> fragmentShaderCode = readFileFromAssets("shaders/template.frag");
    mShader.loadShader(vertexShaderCode.data(), fragmentShaderCode.data());
}

void TemplatePass::initMeshes(const std::map<std::string, renderMesh>& Meshes) {
    mMeshes = Meshes;
}

void TemplatePass::initialize(const std::map<std::string, renderMesh>& Meshes) {
    initShader();
    initMeshes(Meshes);
}

bool TemplatePass::render(const glm::mat4 &p, const glm::mat4 &v, const glm::mat4 &m) {
    mShader.use();
    mShader.setUniformMat4("projection", p);
    mShader.setUniformMat4("view", v);
    mShader.setUniformMat4("model", m);
    draw();
    glUseProgram(0);
    return true;
}

void TemplatePass::draw() {
    GL_CALL(glFrontFace(GL_CCW));
    GL_CALL(glCullFace(GL_BACK));
    GL_CALL(glEnable(GL_CULL_FACE));
    GL_CALL(glEnable(GL_DEPTH_TEST));
    GL_CALL(glEnable(GL_BLEND));
    GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    for (auto &it : mMeshes) {
        it.second.draw(mShader);
    }
}
