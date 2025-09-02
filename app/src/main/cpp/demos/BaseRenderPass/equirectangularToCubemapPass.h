//
// Created by gyp on 2025/7/7.
//
#pragma once

#include "templatePass.h"

class EquirectangularToCubemapPass : public TemplatePass {
public:
    EquirectangularToCubemapPass();
    virtual ~EquirectangularToCubemapPass();


    virtual bool render(const glm::mat4& p, const glm::mat4& v, const glm::mat4& m) override;
    virtual void render(const glm::mat4& p, const glm::mat4& v);
    unsigned int getCaptureFBO() const { return mCaptureFBO;}
    unsigned int getCaptureRbo() const {return mCaptureRBO;}
    unsigned int getEnvCubemap() const {return mEnvCubemap;}
    const glm::mat4 &getCaptureProjection() const {return mCaptureProjection;}
    const glm::mat4 *getCaptureViews() const {return mCaptureViews;}

protected:
    virtual void initShader() override;
    virtual void draw() override;
private:
    void renderCube();
    unsigned int mCaptureFBO = 0;
    unsigned int mCaptureRBO = 0;
    unsigned int mEnvCubemap = 0;

    glm::mat4 mCaptureProjection;
    glm::mat4 mCaptureViews[6];
    unsigned int cubeVAO = 0;
    unsigned int cubeVBO = 0;

};
