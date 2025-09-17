//
// Created by gyp on 2025/9/8.
//
#pragma once
#include "templatePass.h"

class SSAOPass : public TemplatePass  {
public:
    SSAOPass();
    virtual ~SSAOPass();
    virtual bool render(const glm::mat4& p, const glm::mat4& v, const glm::mat4& m) override;
    virtual void render(const glm::mat4& p, const glm::mat4& v);
protected:
    virtual void initShader() override;
    virtual void draw() override;
private:
    float ourLerp(float a, float b, float f)
    {
        return a + f * (b - a);
    }
    renderShader mSSAOLightingShader;
    renderShader mSSAOShader;
    renderShader mSSAOBlurShader;
    unsigned int ssaoFBO, ssaoBlurFBO;
    std::vector<glm::vec3> ssaoKernel;
    std::vector<glm::vec3> ssaoNoise;
    unsigned int noiseTexture;
    unsigned int ssaoColorBuffer, ssaoColorBufferBlur;
public:
    unsigned int getSsaoColorBuffer() const;

    unsigned int getSsaoColorBufferBlur() const {
        return ssaoColorBufferBlur;
    }

private:
    // lighting info
    // -------------
    glm::vec3 lightPos = glm::vec3(2.0, 4.0, -2.0);
    glm::vec3 lightColor = glm::vec3(0.2, 0.2, 0.7);
    int mCurViewport[4] = {0, 0, 0, 0}; // x, y, width, height
    unsigned int quadVAO = 0;
    unsigned int quadVBO;
    void renderQuad();

    renderShader mDebugShader;
    void debugRender();
};
