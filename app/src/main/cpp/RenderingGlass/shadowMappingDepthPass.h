//
// Created by gyp on 2025/7/9.
//
#pragma once

#include "templatePass.h"

class ShadowMappingDepthPass : public TemplatePass {
public:
    ShadowMappingDepthPass();
    virtual ~ShadowMappingDepthPass();


    virtual bool render(const glm::mat4& p, const glm::mat4& v, const glm::mat4& m) override;
    virtual void render(const glm::mat4& p, const glm::mat4& v);
    void debugShadowMapRender();
    const glm::mat4 &getLightSpaceMatrix() const {return lightSpaceMatrix;}
    const glm::vec3 &getLightPos() const {return lightPos;}
    unsigned int getDepthMap() const {return depthMap;}
    unsigned int getPlaneVao() const {return planeVAO;}
    const glm::mat4 &getLightView() const {return lightView;}

    const glm::mat4 &getLightProjection() const {return lightProjection;}
protected:
    virtual void initShader() override;
    virtual void draw() override;

private:
    // lighting info
    // -------------
    glm::vec3 lightPos;


private:
    // create depth texture
    unsigned int depthMap;



private:
    unsigned int depthMapFBO;
    const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
    unsigned int planeVBO,planeVAO;
    float near_plane = 0.00001f, far_plane = 100.f;
    void renderScene(const renderShader &shader);

    renderShader debugShadowMapShader;
    unsigned int quadVAO = 0;
    unsigned int quadVBO;
    void renderQuad();
    glm::mat4 lightSpaceMatrix;

    void renderSphere();
    unsigned int sphereVAO = 0;
    unsigned int indexCount;
    glm::mat4 lightProjection, lightView;

};
