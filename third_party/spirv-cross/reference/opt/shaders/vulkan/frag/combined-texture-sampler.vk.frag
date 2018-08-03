#version 310 es
precision mediump float;
precision highp int;

uniform mediump sampler2D SPIRV_Cross_CombineduTexture0uSampler0;
uniform mediump sampler2D SPIRV_Cross_CombineduTexture1uSampler1;
uniform mediump sampler2D SPIRV_Cross_CombineduTexture1uSampler0;
uniform mediump sampler2D SPIRV_Cross_CombineduTexture0uSampler1;

layout(location = 0) in vec2 vTex;
layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = ((((texture(SPIRV_Cross_CombineduTexture0uSampler0, vTex) + texture(SPIRV_Cross_CombineduTexture1uSampler1, vTex)) + (texture(SPIRV_Cross_CombineduTexture0uSampler0, vTex) + texture(SPIRV_Cross_CombineduTexture1uSampler0, vTex))) + (texture(SPIRV_Cross_CombineduTexture0uSampler1, vTex) + texture(SPIRV_Cross_CombineduTexture1uSampler1, vTex))) + (texture(SPIRV_Cross_CombineduTexture0uSampler0, vTex) + texture(SPIRV_Cross_CombineduTexture0uSampler1, vTex))) + (texture(SPIRV_Cross_CombineduTexture1uSampler0, vTex) + texture(SPIRV_Cross_CombineduTexture1uSampler1, vTex));
}

