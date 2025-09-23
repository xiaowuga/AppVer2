//
// Created by gyp on 2025/7/6.
//
#pragma once

#include "renderMesh.h"
#include <map>


class TemplatePass {
public:
    TemplatePass() = default;
    virtual ~TemplatePass();

    virtual bool render(const glm::mat4& p, const glm::mat4& v, const glm::mat4& m);
    virtual void initialize(const std::map<std::string, renderMesh>& Meshes);
    virtual void render(const glm::mat4& p, const glm::mat4& v) = 0;
protected:

    virtual void initShader();
    virtual void initMeshes(const std::map<std::string, renderMesh>& Meshes);
    virtual void draw();

    renderShader mShader;
    std::map<std::string, renderMesh> mMeshes;
};