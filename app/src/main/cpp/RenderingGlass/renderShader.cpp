#include <iostream>
#include "renderShader.h"
#include "demos/utils.h"

renderShader::renderShader() : mProgram(0) {
}

renderShader::~renderShader() {
    if (mProgram) {
        glDeleteProgram(mProgram);
    }
}

bool renderShader::checkCompileErrors(GLuint shader, std::string type) {
    GLint success = 0;
    GLchar infoLog[1024] = { 0 };
    if (type != "PROGRAM") {
        GL_CALL(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
        if (!success) {
            GL_CALL(glGetShaderInfoLog(shader, 1024, nullptr, infoLog));
            errorf("SHADER_COMPILATION_ERROR of type: %s %s", type.c_str(), infoLog);
            return false;
        }
    } else {
        GL_CALL(glGetProgramiv(shader, GL_LINK_STATUS, &success));
        if (!success) {
            GL_CALL(glGetProgramInfoLog(shader, 1024, nullptr, infoLog));
            errorf("PROGRAM_LINKING_ERROR of type: %s %s", type.c_str(), infoLog);
            return false;
        }
    }
    return true;
}

bool renderShader::loadShader(const char* vertexShaderCode, const char* fragmentShaderCode) {
    int maxVertexUniform, maxFragmentUniform;
    GL_CALL(glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &maxVertexUniform));
    GL_CALL(glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &maxFragmentUniform));
    //infof("maxVertexUniform:%d, maxFragmentUniform:%d", maxVertexUniform, maxFragmentUniform);

    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    GL_CALL(glShaderSource(vertex, 1, &vertexShaderCode, nullptr));
    GL_CALL(glCompileShader(vertex));
    if (!checkCompileErrors(vertex, "VERTEX")) {
        return false;
    }
    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    GL_CALL(glShaderSource(fragment, 1, &fragmentShaderCode, nullptr));
    GL_CALL(glCompileShader(fragment));
    if (!checkCompileErrors(fragment, "FRAGMENT")) {
        return false;
    }
    mProgram = glCreateProgram();
    GL_CALL(glAttachShader(mProgram, vertex));
    GL_CALL(glAttachShader(mProgram, fragment));
    GL_CALL(glLinkProgram(mProgram));
    if (!checkCompileErrors(mProgram, "PROGRAM")) {
        return false;
    }
    GL_CALL(glDeleteShader(vertex));
    GL_CALL(glDeleteShader(fragment));
    return true;
}

void renderShader::use() const {
    GL_CALL(glUseProgram(mProgram));
}

GLuint renderShader::id() const {
    return mProgram;
}

void renderShader::setUniformBool(const std::string& name, bool value) const {
    GL_CALL(glUniform1i(glGetUniformLocation(mProgram, name.c_str()), (int)value));
}

void renderShader::setUniformInt(const std::string& name, int value) const {
    GL_CALL(glUniform1i(glGetUniformLocation(mProgram, name.c_str()), value));
}

void renderShader::setUniformFloat(const std::string& name, float value) const {
    GL_CALL(glUniform1f(glGetUniformLocation(mProgram, name.c_str()), value));
}

void renderShader::setUniformVec2(const std::string& name, const glm::vec2& value) const {
    GL_CALL(glUniform2fv(glGetUniformLocation(mProgram, name.c_str()), 1, &value[0]));
}

void renderShader::setUniformVec2(const std::string& name, float x, float y) const {
    GL_CALL(glUniform2f(glGetUniformLocation(mProgram, name.c_str()), x, y));
}

void renderShader::setUniformVec3(const std::string& name, const glm::vec3& value) const {
    GL_CALL(glUniform3fv(glGetUniformLocation(mProgram, name.c_str()), 1, &value[0]));
}

void renderShader::setUniformVec3(const std::string& name, float x, float y, float z) const {
    GL_CALL(glUniform3f(glGetUniformLocation(mProgram, name.c_str()), x, y, z));
}

void renderShader::setUniformVec4(const std::string& name, const glm::vec4& value) const {
    GL_CALL(glUniform4fv(glGetUniformLocation(mProgram, name.c_str()), 1, &value[0]));
}

void renderShader::setUniformVec4(const std::string& name, float x, float y, float z, float w) const {
    GL_CALL(glUniform4f(glGetUniformLocation(mProgram, name.c_str()), x, y, z, w));
}

void renderShader::setUniformMat2(const std::string& name, const glm::mat2& mat) const {
    GL_CALL(glUniformMatrix2fv(glGetUniformLocation(mProgram, name.c_str()), 1, GL_FALSE, &mat[0][0]));
}

void renderShader::setUniformMat3(const std::string& name, const glm::mat3& mat) const {
    GL_CALL(glUniformMatrix3fv(glGetUniformLocation(mProgram, name.c_str()), 1, GL_FALSE, &mat[0][0]));
}

void renderShader::setUniformMat4(const std::string& name, const glm::mat4& mat) const {
    GL_CALL(glUniformMatrix4fv(glGetUniformLocation(mProgram, name.c_str()), 1, GL_FALSE, &mat[0][0]));
}

GLuint renderShader::getAttribLocation(const std::string& name) const {
    return glGetAttribLocation(mProgram, name.c_str());
}
