#version 310 es
precision mediump float;

layout(set = 0, binding = 0) uniform mediump sampler uSampler;
layout(set = 0, binding = 1) uniform mediump texture2D uTexture;
layout(set = 0, binding = 2) uniform mediump texture3D uTexture3D;
layout(set = 0, binding = 3) uniform mediump textureCube uTextureCube;
layout(set = 0, binding = 4) uniform mediump texture2DArray uTextureArray;

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec2 vTex;
layout(location = 1) in vec3 vTex3;

vec4 sample_func(mediump sampler samp, vec2 uv)
{
    return texture(sampler2D(uTexture, samp), uv);
}

vec4 sample_func_dual(mediump sampler samp, mediump texture2D tex, vec2 uv)
{
    return texture(sampler2D(tex, samp), uv);
}

void main()
{
    vec2 off = 1.0 / vec2(textureSize(sampler2D(uTexture, uSampler), 0));
    vec2 off2 = 1.0 / vec2(textureSize(sampler2D(uTexture, uSampler), 1));

    vec4 c0 = sample_func(uSampler, vTex + off + off2);
    vec4 c1 = sample_func_dual(uSampler, uTexture, vTex + off + off2);
    vec4 c2 = texture(sampler2DArray(uTextureArray, uSampler), vTex3);
    vec4 c3 = texture(samplerCube(uTextureCube, uSampler), vTex3);
    vec4 c4 = texture(sampler3D(uTexture3D, uSampler), vTex3);

    FragColor = c0 + c1 + c2 + c3 + c4;
}
