#version 450
#extension GL_ARB_sparse_texture2 : require
#extension GL_ARB_sparse_texture_clamp : require

layout(set = 0, binding = 0) uniform sampler2D uSamp;
layout(set = 0, binding = 1) uniform sampler2DMS uSampMS;
layout(set = 0, binding = 2, rgba8) uniform image2D uImage;
layout(set = 0, binding = 3, rgba8) uniform image2DMS uImageMS;
layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec2 vUV;

void main()
{
    vec4 texel;
    bool ret;

    ret = sparseTexelsResidentARB(sparseTextureARB(uSamp, vUV, texel));
    ret = sparseTexelsResidentARB(sparseTextureARB(uSamp, vUV, texel, 1.1));
    ret = sparseTexelsResidentARB(sparseTextureLodARB(uSamp, vUV, 1.0, texel));
    ret = sparseTexelsResidentARB(sparseTextureOffsetARB(uSamp, vUV, ivec2(1, 1), texel));
    ret = sparseTexelsResidentARB(sparseTextureOffsetARB(uSamp, vUV, ivec2(2, 2), texel, 0.5));
    ret = sparseTexelsResidentARB(sparseTexelFetchARB(uSamp, ivec2(vUV), 1, texel));
    ret = sparseTexelsResidentARB(sparseTexelFetchARB(uSampMS, ivec2(vUV), 2, texel));
    ret = sparseTexelsResidentARB(sparseTexelFetchOffsetARB(uSamp, ivec2(vUV), 1, ivec2(2, 3), texel));
    ret = sparseTexelsResidentARB(sparseTextureLodOffsetARB(uSamp, vUV, 1.5, ivec2(2, 3), texel));
    ret = sparseTexelsResidentARB(sparseTextureGradARB(uSamp, vUV, vec2(1.0), vec2(3.0), texel));
    ret = sparseTexelsResidentARB(sparseTextureGradOffsetARB(uSamp, vUV, vec2(1.0), vec2(3.0), ivec2(-2, -3), texel));
    ret = sparseTexelsResidentARB(sparseTextureClampARB(uSamp, vUV, 4.0, texel));
    ret = sparseTexelsResidentARB(sparseImageLoadARB(uImage, ivec2(vUV), texel));
    ret = sparseTexelsResidentARB(sparseImageLoadARB(uImageMS, ivec2(vUV), 1, texel));
}
