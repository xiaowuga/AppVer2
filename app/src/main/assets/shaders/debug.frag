#version 320 es
precision mediump float;
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
//uniform sampler2D depthMap;
//uniform sampler2D texNoise;

void main()
{
    // 输出 gPosition 的 xyz 到 rgb
    // FragColor = vec4(texture(gPosition, TexCoords).xyz, 1.0);

    // 输出 gNormal 的 xyz 到 rgb
     //FragColor = vec4(texture(gNormal, TexCoords).rgb, 1.0);
     FragColor = vec4(texture(gPosition, TexCoords).rgb, 1.0);
     //float depthValue = texture(depthMap, TexCoords).r;

     //FragColor = vec4(vec3(depthValue), 1.0);
     //FragColor = vec4(1.0,1.0,1.0, 1.0);

    // 输出 texNoise 的 rgb 到 rgb
    // FragColor = vec4(texture(texNoise, TexCoords).rgb, 1.0);

    // 你可以根据需要取消注释对应行
    //FragColor = vec4(texture(gPosition, TexCoords).xyz, 1.0);
    //FragColor = vec4(0.0, 1.0, 1.0, 1.0);
}
