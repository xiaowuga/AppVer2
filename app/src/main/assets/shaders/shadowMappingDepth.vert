#version 320 es
precision highp float;
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout(location = 7) in vec4 instanceMatrix0; // 第一列
layout(location = 8) in vec4 instanceMatrix1; // 第二列
layout(location = 9) in vec4 instanceMatrix2; // 第三列
layout(location = 10) in vec4 instanceMatrix3; // 第四列
uniform mat4 lightSpaceMatrix;
uniform mat4 model;
uniform bool isNotInstanced; // 是否不使用实例化渲染
void main()
{
    mat4 modelMatrix = mat4(instanceMatrix0, instanceMatrix1, instanceMatrix2, instanceMatrix3);
    mat4 finalModel = !isNotInstanced? model * modelMatrix : model; // 合并变换
    gl_Position = lightSpaceMatrix * finalModel * vec4(aPos, 1.0);
}