#version 320 es
precision mediump float;

layout (location = 0) out vec4 gPosition;


in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

void main()
{

    // store the fragment position vector in the first gbuffer texture
    gPosition = vec4(FragPos,1.0);
    // also store the per-fragment normals into the gbuffer
    //gNormal = vec4(normalize(Normal),1.0);
    //gNormal = vec4(1.0,1.0,0.0,1.0);
    // and the diffuse per-fragment color
    //gAlbedo = vec4(0.5,0.5,0.5, 1.0);

}