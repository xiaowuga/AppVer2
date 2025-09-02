//
// Created by gyp on 2025/7/7.
//
#pragma once

#include "templatePass.h"

class BrdfPass : public TemplatePass {
public:
    BrdfPass();
    virtual ~BrdfPass();


    virtual bool render(const glm::mat4& p, const glm::mat4& v, const glm::mat4& m) override;
    virtual void render(const glm::mat4& p, const glm::mat4& v);
    unsigned int getBrdfLUTTexture() const {return mBrdfLUTTexture;}
    Shader getShader() const {return mShader;};
    void renderQuad();
protected:
    virtual void initShader() override;
    virtual void draw() override;
private:

    unsigned int mBrdfLUTTexture=0;
    unsigned int quadVAO = 0;
    unsigned int quadVBO = 0;

};
