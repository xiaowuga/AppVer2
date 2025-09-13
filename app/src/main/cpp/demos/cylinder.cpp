#include"cylinder.h"
#include<cmath>

#define RADIAN(x) ((x) * 3.1415926f / 180.0f)

Shader Cylinder::mShader;


Cylinder::Cylinder(float height,float radius):mRadius(radius),mHeight(height){
    mColor = {0.7f, 0.7f, 0.7f};
}
Cylinder::~Cylinder() = default;

bool Cylinder::initShader() {
    static bool init = false;
    if (init) return true;

    const GLchar* vertex_shader_glsl = R"_(
        #version 320 es
        precision highp float;
        layout(location = 0) in vec3 position;
        uniform mat4 projection;
        uniform mat4 view;
        uniform mat4 model;
        out vec3 outPosition;
        void main() {
            outPosition = position;
            gl_Position = projection * view * model * vec4(position, 1.0);
        }
    )_";

    const GLchar* fragment_shader_glsl = R"_(
        #version 320 es
        precision mediump float;
        uniform vec3 color;
        in vec3 outPosition;
        out vec4 FragColor;
        void main() {
            FragColor = vec4(color, 1.0);
        }
    )_";

    if (!mShader.loadShader(vertex_shader_glsl, fragment_shader_glsl)) return false;
    init = true;
    return true;
}

void Cylinder::initialize() {
    initShader();

    mVertices.clear();
    mIndices.clear();
    float halfH = mHeight / 2.0f;
    mBottomCenter = glm::vec3(0.0f, 0.0f, -halfH);
    mTopCenter = glm::vec3(0.0f, 0.0f, +halfH);
    // 顶面/底面圆周点
    for (int i = 0; i <= mSegments; ++i) {
        float angle = RADIAN(i * 360.0f / mSegments);
        float x = mRadius * cos(angle);
        float y = mRadius * sin(angle);
        float z1 = -halfH;
        float z2 = +halfH;
        // 底圆点
        mVertices.push_back(x);
        mVertices.push_back(y);
        mVertices.push_back(z1);
        // 顶圆点
        mVertices.push_back(x);
        mVertices.push_back(y);
        mVertices.push_back(z2);
    }
    int vertexPerCircle = (mSegments + 1);
    // 顶盖和底盖中心点
    int bottomCenterIndex = mVertices.size() / 3;
    mVertices.push_back(0.0f);
    mVertices.push_back(0.0f);
    mVertices.push_back(-halfH);
    int topCenterIndex = mVertices.size() / 3;
    mVertices.push_back(0.0f);
    mVertices.push_back(0.0f);
    mVertices.push_back(+halfH);

    // 构建索引
    for (int i = 0; i < mSegments; ++i) {
        int i0 = i * 2;
        int i1 = i0 + 2;

        // 侧面（两个三角形）
        mIndices.push_back(i0);
        mIndices.push_back(i0 + 1);
        mIndices.push_back(i1);

        mIndices.push_back(i0 + 1);
        mIndices.push_back(i1 + 1);
        mIndices.push_back(i1);

        // 底面
        mIndices.push_back(bottomCenterIndex);
        mIndices.push_back(i0);
        mIndices.push_back(i1);

        // 顶面
        mIndices.push_back(topCenterIndex);
        mIndices.push_back(i1 + 1);
        mIndices.push_back(i0 + 1);
    }

    GLuint VBO = 0, EBO = 0;
    glGenVertexArrays(1, &mVAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(mVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, mVertices.size() * sizeof(float), mVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndices.size() * sizeof(GLuint), mIndices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
}

void Cylinder::render(const glm::mat4& p, const glm::mat4& v, const glm::mat4& m) {
    mShader.use();
    mShader.setUniformMat4("projection", p);
    mShader.setUniformMat4("view", v);
    mShader.setUniformMat4("model", m);
    mShader.setUniformVec3("color", mColor);

    glBindVertexArray(mVAO);
    glDrawElements(GL_TRIANGLES, mIndices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

glm::vec3 Cylinder::getDirectionVector(const glm::mat4& m) {
    glm::vec3 top = glm::vec3(m * glm::vec4(mTopCenter, 1.0f));
    glm::vec3 bottom = glm::vec3(m * glm::vec4(mBottomCenter, 1.0f));
    return glm::normalize(top - bottom);
}

void Cylinder::setColor(const glm::vec3& color) {
    mColor = color;
}

void Cylinder::setColor(float x, float y, float z) {
    mColor = glm::vec3(x, y, z);
}

std::vector<glm::vec3> Cylinder::getPoints() {
    return {mBottomCenter, mTopCenter};
}
void Cylinder::setHeight(float height){
    mHeight = height;
    initialize(); // 重新构建几何
}
