#version 450 core

#extension GL_ARB_sparse_texture2: enable
#extension GL_ARB_sparse_texture_clamp: enable
#extension GL_AMD_texture_gather_bias_lod: enable

layout(set = 0, binding =  0) uniform sampler1D            s1D;
layout(set = 0, binding =  1) uniform sampler2D            s2D;
layout(set = 0, binding =  2) uniform sampler3D            s3D;
layout(set = 0, binding =  3) uniform sampler2DRect        s2DRect;
layout(set = 0, binding =  4) uniform samplerCube          sCube;
layout(set = 0, binding =  5) uniform samplerBuffer        sBuffer;
layout(set = 0, binding =  6) uniform sampler2DMS          s2DMS;
layout(set = 0, binding =  7) uniform sampler1DArray       s1DArray;
layout(set = 0, binding =  8) uniform sampler2DArray       s2DArray;
layout(set = 0, binding =  9) uniform samplerCubeArray     sCubeArray;
layout(set = 0, binding = 10) uniform sampler2DMSArray     s2DMSArray;

layout(set = 0, binding = 11) uniform sampler1DShadow          s1DShadow;
layout(set = 0, binding = 12) uniform sampler2DShadow          s2DShadow;
layout(set = 0, binding = 13) uniform sampler2DRectShadow      s2DRectShadow;
layout(set = 0, binding = 14) uniform samplerCubeShadow        sCubeShadow;
layout(set = 0, binding = 15) uniform sampler1DArrayShadow     s1DArrayShadow;
layout(set = 0, binding = 16) uniform sampler2DArrayShadow     s2DArrayShadow;
layout(set = 0, binding = 17) uniform samplerCubeArrayShadow   sCubeArrayShadow;

layout(set = 1, binding =  0) layout(rgba16f) uniform image1D          i1D;
layout(set = 1, binding =  1) layout(rgba16f) uniform image2D          i2D;
layout(set = 1, binding =  2) layout(rgba16f) uniform image3D          i3D;
layout(set = 1, binding =  3) layout(rgba16f) uniform image2DRect      i2DRect;
layout(set = 1, binding =  4) layout(rgba16f) uniform imageCube        iCube;
layout(set = 1, binding =  5) layout(rgba16f) uniform image1DArray     i1DArray;
layout(set = 1, binding =  6) layout(rgba16f) uniform image2DArray     i2DArray;
layout(set = 1, binding =  7) layout(rgba16f) uniform imageCubeArray   iCubeArray;
layout(set = 1, binding =  8) layout(rgba16f) uniform imageBuffer      iBuffer;
layout(set = 1, binding =  9) layout(rgba16f) uniform image2DMS        i2DMS;
layout(set = 1, binding = 10) layout(rgba16f) uniform image2DMSArray   i2DMSArray;

layout(set = 2, binding =  0) uniform texture1D           t1D;
layout(set = 2, binding =  1) uniform texture2D           t2D;
layout(set = 2, binding =  2) uniform texture3D           t3D;
layout(set = 2, binding =  3) uniform texture2DRect       t2DRect;
layout(set = 2, binding =  4) uniform textureCube         tCube;
layout(set = 2, binding =  5) uniform texture1DArray      t1DArray;
layout(set = 2, binding =  6) uniform texture2DArray      t2DArray;
layout(set = 2, binding =  7) uniform textureCubeArray    tCubeArray;
layout(set = 2, binding =  8) uniform textureBuffer       tBuffer;
layout(set = 2, binding =  9) uniform texture2DMS         t2DMS;
layout(set = 2, binding = 10) uniform texture2DMSArray    t2DMSArray;

layout(set = 2, binding = 11) uniform sampler s;
layout(set = 2, binding = 12) uniform samplerShadow sShadow;

layout(set = 3, binding = 0, input_attachment_index = 0) uniform subpassInput   subpass;
layout(set = 3, binding = 1, input_attachment_index = 0) uniform subpassInputMS subpassMS;

layout(location =  0) in float c1;
layout(location =  1) in vec2  c2;
layout(location =  2) in vec3  c3;
layout(location =  3) in vec4  c4;

layout(location =  4) in float compare;
layout(location =  5) in float lod;
layout(location =  6) in float bias;
layout(location =  7) in float lodClamp;

layout(location =  8) in float dPdxy1;
layout(location =  9) in vec2  dPdxy2;
layout(location = 10) in vec3  dPdxy3;

const int   offset1 = 1;
const ivec2 offset2 = ivec2(1);
const ivec3 offset3 = ivec3(1);
const ivec2 offsets[4] = { offset2, offset2, offset2, offset2 };

layout(location = 0) out vec4 fragColor;

vec4 testTexture()
{
    vec4 texel = vec4(0.0);

    texel   += texture(s1D, c1);
    texel   += texture(s2D, c2);
    texel   += texture(s3D, c3);
    texel   += texture(sCube, c3);
    texel.x += texture(s1DShadow, c3);
    texel.x += texture(s2DShadow, c3);
    texel.x += texture(sCubeShadow, c4);
    texel   += texture(s1DArray, c2);
    texel   += texture(s2DArray, c3);
    texel   += texture(sCubeArray, c4);
    texel.x += texture(s1DArrayShadow, c3);
    texel.x += texture(s2DArrayShadow, c4);
    texel   += texture(s2DRect, c2);
    texel.x += texture(s2DRectShadow, c3);
    texel.x += texture(sCubeArrayShadow, c4, compare);

    return texel;
}

vec4 testTextureProj()
{
    vec4 texel = vec4(0.0);

    texel   += textureProj(s1D, c2);
    texel   += textureProj(s1D, c4);
    texel   += textureProj(s2D, c3);
    texel   += textureProj(s2D, c4);
    texel   += textureProj(s3D, c4);
    texel.x += textureProj(s1DShadow, c4);
    texel.x += textureProj(s2DShadow, c4);
    texel   += textureProj(s2DRect, c3);
    texel   += textureProj(s2DRect, c4);
    texel.x += textureProj(s2DRectShadow, c4);

    return texel;
}

vec4 testTextureLod()
{
    vec4 texel = vec4(0.0);

    texel   += textureLod(s1D, c1, lod);
    texel   += textureLod(s2D, c2, lod);
    texel   += textureLod(s3D, c3, lod);
    texel   += textureLod(sCube, c3, lod);
    texel.x += textureLod(s1DShadow, c3, lod);
    texel.x += textureLod(s2DShadow, c3, lod);
    texel   += textureLod(s1DArray, c2, lod);
    texel   += textureLod(s2DArray, c3, lod);
    texel.x += textureLod(s1DArrayShadow, c3, lod);
    texel   += textureLod(sCubeArray, c4, lod);

    return texel;
}

vec4 testTextureOffset()
{
    vec4 texel = vec4(0.0);

    texel   += textureOffset(s1D, c1, offset1);
    texel   += textureOffset(s2D, c2, offset2);
    texel   += textureOffset(s3D, c3, offset3);
    texel   += textureOffset(s2DRect, c2, offset2);
    texel.x += textureOffset(s2DRectShadow, c3, offset2);
    texel.x += textureOffset(s1DShadow, c3, offset1);
    texel.x += textureOffset(s2DShadow, c3, offset2);
    texel   += textureOffset(s1DArray, c2, offset1);
    texel   += textureOffset(s2DArray, c3, offset2);
    texel.x += textureOffset(s1DArrayShadow, c3, offset1);
    texel.x += textureOffset(s2DArrayShadow, c4, offset2);

    return texel;
}

vec4 testTextureProjOffset()
{
    vec4 texel = vec4(0.0);

    texel   += textureProjOffset(s1D, c2, offset1);
    texel   += textureProjOffset(s1D, c4, offset1);
    texel   += textureProjOffset(s2D, c3, offset2);
    texel   += textureProjOffset(s2D, c4, offset2);
    texel   += textureProjOffset(s3D, c4, offset3);
    texel   += textureProjOffset(s2DRect, c3, offset2);
    texel   += textureProjOffset(s2DRect, c4, offset2);
    texel.x += textureProjOffset(s2DRectShadow, c4, offset2);
    texel.x += textureProjOffset(s1DShadow, c4, offset1);
    texel.x += textureProjOffset(s2DShadow, c4, offset2);

    return texel;
}

vec4 testTextureLodOffset()
{
    vec4 texel = vec4(0.0);

    texel   += textureLodOffset(s1D, c1, lod, offset1);
    texel   += textureLodOffset(s2D, c2, lod, offset2);
    texel   += textureLodOffset(s3D, c3, lod, offset3);
    texel.x += textureLodOffset(s1DShadow, c3, lod, offset1);
    texel.x += textureLodOffset(s2DShadow, c3, lod, offset2);
    texel   += textureLodOffset(s1DArray, c2, lod, offset1);
    texel   += textureLodOffset(s2DArray, c3, lod, offset2);
    texel.x += textureLodOffset(s1DArrayShadow, c3, lod, offset1);

    return texel;
}

vec4 testTextureProjLodOffset()
{
    vec4 texel = vec4(0.0);

    texel   += textureProjLodOffset(s1D, c2, lod, offset1);
    texel   += textureProjLodOffset(s1D, c4, lod, offset1);
    texel   += textureProjLodOffset(s2D, c3, lod, offset2);
    texel   += textureProjLodOffset(s2D, c4, lod, offset2);
    texel   += textureProjLodOffset(s3D, c4, lod, offset3);
    texel.x += textureProjLodOffset(s1DShadow, c4, lod, offset1);
    texel.x += textureProjLodOffset(s2DShadow, c4, lod, offset2);

    return texel;
}

vec4 testTexelFetch()
{
    vec4 texel = vec4(0.0);

    texel   += texelFetch(s1D, int(c1), int(lod));
    texel   += texelFetch(s2D, ivec2(c2), int(lod));
    texel   += texelFetch(s3D, ivec3(c3), int(lod));
    texel   += texelFetch(s2DRect, ivec2(c2));
    texel   += texelFetch(s1DArray, ivec2(c2), int(lod));
    texel   += texelFetch(s2DArray, ivec3(c3), int(lod));
    texel   += texelFetch(sBuffer, int(c1));
    texel   += texelFetch(s2DMS, ivec2(c2), 1);
    texel   += texelFetch(s2DMSArray, ivec3(c3), 2);

    return texel;
}

vec4 testTexelFetchOffset()
{
    vec4 texel = vec4(0.0);

    texel   += texelFetchOffset(s1D, int(c1), int(lod), offset1);
    texel   += texelFetchOffset(s2D, ivec2(c2), int(lod), offset2);
    texel   += texelFetchOffset(s3D, ivec3(c3), int(lod), offset3);
    texel   += texelFetchOffset(s2DRect, ivec2(c2), offset2);
    texel   += texelFetchOffset(s1DArray, ivec2(c2), int(lod), offset1);
    texel   += texelFetchOffset(s2DArray, ivec3(c3), int(lod), offset2);

    return texel;
}

vec4 testTextureGrad()
{
    vec4 texel = vec4(0.0);

    texel   += textureGrad(s1D, c1, dPdxy1, dPdxy1);
    texel   += textureGrad(s2D, c2, dPdxy2, dPdxy2);
    texel   += textureGrad(s3D, c3, dPdxy3, dPdxy3);
    texel   += textureGrad(sCube, c3, dPdxy3, dPdxy3);
    texel   += textureGrad(s2DRect, c2, dPdxy2, dPdxy2);
    texel.x += textureGrad(s2DRectShadow, c3, dPdxy2, dPdxy2);
    texel.x += textureGrad(s1DShadow, c3, dPdxy1, dPdxy1);
    texel.x += textureGrad(s2DShadow, c3, dPdxy2, dPdxy2);
    texel.x += textureGrad(sCubeShadow, c4, dPdxy3, dPdxy3);
    texel   += textureGrad(s1DArray, c2, dPdxy1, dPdxy1);
    texel   += textureGrad(s2DArray, c3, dPdxy2, dPdxy2);
    texel.x += textureGrad(s1DArrayShadow, c3, dPdxy1, dPdxy1);
    texel.x += textureGrad(s2DArrayShadow, c4, dPdxy2, dPdxy2);
    texel   += textureGrad(sCubeArray, c4, dPdxy3, dPdxy3);

    return texel;
}

vec4 testTextureGradOffset()
{
    vec4 texel = vec4(0.0);

    texel   += textureGradOffset(s1D, c1, dPdxy1, dPdxy1, offset1);
    texel   += textureGradOffset(s2D, c2, dPdxy2, dPdxy2, offset2);
    texel   += textureGradOffset(s3D, c3, dPdxy3, dPdxy3, offset3);
    texel   += textureGradOffset(s2DRect, c2, dPdxy2, dPdxy2, offset2);
    texel.x += textureGradOffset(s2DRectShadow, c3, dPdxy2, dPdxy2, offset2);
    texel.x += textureGradOffset(s1DShadow, c3, dPdxy1, dPdxy1, offset1);
    texel.x += textureGradOffset(s2DShadow, c3, dPdxy2, dPdxy2, offset2);
    texel   += textureGradOffset(s1DArray, c2, dPdxy1, dPdxy1, offset1);
    texel   += textureGradOffset(s2DArray, c3, dPdxy2, dPdxy2, offset2);
    texel.x += textureGradOffset(s1DArrayShadow, c3, dPdxy1, dPdxy1, offset1);
    texel.x += textureGradOffset(s2DArrayShadow, c4, dPdxy2, dPdxy2, offset2);

    return texel;
}

vec4 testTextureProjGrad()
{
    vec4 texel = vec4(0.0);

    texel   += textureProjGrad(s1D, c2, dPdxy1, dPdxy1);
    texel   += textureProjGrad(s1D, c4, dPdxy1, dPdxy1);
    texel   += textureProjGrad(s2D, c3, dPdxy2, dPdxy2);
    texel   += textureProjGrad(s2D, c4, dPdxy2, dPdxy2);
    texel   += textureProjGrad(s3D, c4, dPdxy3, dPdxy3);
    texel   += textureProjGrad(s2DRect, c3, dPdxy2, dPdxy2);
    texel   += textureProjGrad(s2DRect, c4, dPdxy2, dPdxy2);
    texel.x += textureProjGrad(s2DRectShadow, c4, dPdxy2, dPdxy2);
    texel.x += textureProjGrad(s1DShadow, c4, dPdxy1, dPdxy1);
    texel.x += textureProjGrad(s2DShadow, c4, dPdxy2, dPdxy2);

    return texel;
}

vec4 testTextureProjGradoffset()
{
    vec4 texel = vec4(0.0);

    texel   += textureProjGradOffset(s1D, c2, dPdxy1, dPdxy1, offset1);
    texel   += textureProjGradOffset(s1D, c4, dPdxy1, dPdxy1, offset1);
    texel   += textureProjGradOffset(s2D, c3, dPdxy2, dPdxy2, offset2);
    texel   += textureProjGradOffset(s2D, c4, dPdxy2, dPdxy2, offset2);
    texel   += textureProjGradOffset(s2DRect, c3, dPdxy2, dPdxy2, offset2);
    texel   += textureProjGradOffset(s2DRect, c4, dPdxy2, dPdxy2, offset2);
    texel.x += textureProjGradOffset(s2DRectShadow, c4, dPdxy2, dPdxy2, offset2);
    texel   += textureProjGradOffset(s3D, c4, dPdxy3, dPdxy3, offset3);
    texel.x += textureProjGradOffset(s1DShadow, c4, dPdxy1, dPdxy1, offset1);
    texel.x += textureProjGradOffset(s2DShadow, c4, dPdxy2, dPdxy2, offset2);

    return texel;
}

vec4 testTextureGather()
{
    vec4 texel = vec4(0.0);

    texel   += textureGather(s2D, c2, 0);
    texel   += textureGather(s2DArray, c3, 0);
    texel   += textureGather(sCube, c3, 0);
    texel   += textureGather(sCubeArray, c4, 0);
    texel   += textureGather(s2DRect, c2, 0);
    texel   += textureGather(s2DShadow, c2, compare);
    texel   += textureGather(s2DArrayShadow, c3, compare);
    texel   += textureGather(sCubeShadow, c3, compare);
    texel   += textureGather(sCubeArrayShadow, c4, compare);
    texel   += textureGather(s2DRectShadow, c2, compare);

    return texel;
}

vec4 testTextureGatherOffset()
{
    vec4 texel = vec4(0.0);

    texel   += textureGatherOffset(s2D, c2, offset2, 0);
    texel   += textureGatherOffset(s2DArray, c3, offset2, 0);
    texel   += textureGatherOffset(s2DRect, c2, offset2, 0);
    texel   += textureGatherOffset(s2DShadow, c2, compare, offset2);
    texel   += textureGatherOffset(s2DArrayShadow, c3, compare, offset2);
    texel   += textureGatherOffset(s2DRectShadow, c2, compare, offset2);

    return texel;
}

vec4 testTextureGatherOffsets()
{
    vec4 texel = vec4(0.0);

    texel   += textureGatherOffsets(s2D, c2, offsets, 0);
    texel   += textureGatherOffsets(s2DArray, c3, offsets, 0);
    texel   += textureGatherOffsets(s2DRect, c2, offsets, 0);
    texel   += textureGatherOffsets(s2DShadow, c2, compare, offsets);
    texel   += textureGatherOffsets(s2DArrayShadow, c3, compare, offsets);
    texel   += textureGatherOffsets(s2DRectShadow, c2, compare, offsets);

    return texel;
}

vec4 testTextureGatherLod()
{
    vec4 texel = vec4(0.0);

    texel   += textureGatherLodAMD(s2D, c2, lod, 0);
    texel   += textureGatherLodAMD(s2DArray, c3, lod, 0);
    texel   += textureGatherLodAMD(sCube, c3, lod, 0);
    texel   += textureGatherLodAMD(sCubeArray, c4, lod, 0);

    return texel;
}

vec4 testTextureGatherLodOffset()
{
    vec4 texel = vec4(0.0);

    texel   += textureGatherLodOffsetAMD(s2D, c2, lod, offset2, 0);
    texel   += textureGatherLodOffsetAMD(s2DArray, c3, lod, offset2, 0);

    return texel;
}

vec4 testTextureGatherLodOffsets()
{
    vec4 texel = vec4(0.0);

    texel   += textureGatherLodOffsetsAMD(s2D, c2, lod, offsets, 0);
    texel   += textureGatherLodOffsetsAMD(s2DArray, c3, lod, offsets, 0);

    return texel;
}

ivec4 testTextureSize()
{
    ivec4 size = ivec4(0);

    size.x      += textureSize(s1D, int(lod));
    size.xy     += textureSize(s2D, int(lod));
    size.xyz    += textureSize(s3D, int(lod));
    size.xy     += textureSize(sCube, int(lod));
    size.x      += textureSize(s1DShadow, int(lod));
    size.xy     += textureSize(s2DShadow, int(lod));
    size.xy     += textureSize(sCubeShadow, int(lod));
    size.xyz    += textureSize(sCubeArray, int(lod));
    size.xyz    += textureSize(sCubeArrayShadow, int(lod));
    size.xy     += textureSize(s2DRect);
    size.xy     += textureSize(s2DRectShadow);
    size.xy     += textureSize(s1DArray, int(lod));
    size.xyz    += textureSize(s2DArray, int(lod));
    size.xy     += textureSize(s1DArrayShadow, int(lod));
    size.xyz    += textureSize(s2DArrayShadow, int(lod));
    size.x      += textureSize(sBuffer);
    size.xy     += textureSize(s2DMS);
    size.xyz    += textureSize(s2DMSArray);

    return size;
}

vec2 testTextureQueryLod()
{
    vec2 lod = vec2(0.0);

    lod  += textureQueryLod(s1D, c1);
    lod  += textureQueryLod(s2D, c2);
    lod  += textureQueryLod(s3D, c3);
    lod  += textureQueryLod(sCube, c3);
    lod  += textureQueryLod(s1DArray, c1);
    lod  += textureQueryLod(s2DArray, c2);
    lod  += textureQueryLod(sCubeArray, c3);
    lod  += textureQueryLod(s1DShadow, c1);
    lod  += textureQueryLod(s2DShadow, c2);
    lod  += textureQueryLod(sCubeArrayShadow, c3);
    lod  += textureQueryLod(s1DArrayShadow, c1);
    lod  += textureQueryLod(s2DArrayShadow, c2);
    lod  += textureQueryLod(sCubeArrayShadow, c3);

    return lod;
}

int testTextureQueryLevels()
{
    int levels = 0;

    levels  += textureQueryLevels(s1D);
    levels  += textureQueryLevels(s2D);
    levels  += textureQueryLevels(s3D);
    levels  += textureQueryLevels(sCube);
    levels  += textureQueryLevels(s1DShadow);
    levels  += textureQueryLevels(s2DShadow);
    levels  += textureQueryLevels(sCubeShadow);
    levels  += textureQueryLevels(sCubeArray);
    levels  += textureQueryLevels(sCubeArrayShadow);
    levels  += textureQueryLevels(s1DArray);
    levels  += textureQueryLevels(s2DArray);
    levels  += textureQueryLevels(s1DArrayShadow);
    levels  += textureQueryLevels(s2DArrayShadow);

    return levels;
}

int testTextureSamples()
{
    int samples = 0;

    samples += textureSamples(s2DMS);
    samples += textureSamples(s2DMSArray);

    return samples;
}

vec4 testImageLoad()
{
    vec4 texel = vec4(0.0);

    texel += imageLoad(i1D, int(c1));
    texel += imageLoad(i2D, ivec2(c2));
    texel += imageLoad(i3D, ivec3(c3));
    texel += imageLoad(i2DRect, ivec2(c2));
    texel += imageLoad(iCube, ivec3(c3));
    texel += imageLoad(iBuffer, int(c1));
    texel += imageLoad(i1DArray, ivec2(c2));
    texel += imageLoad(i2DArray, ivec3(c3));
    texel += imageLoad(iCubeArray, ivec3(c3));
    texel += imageLoad(i2DMS, ivec2(c2), 1);
    texel += imageLoad(i2DMSArray, ivec3(c3), 1);

    return texel;
}

void testImageStore(vec4 data)
{
    imageStore(i1D, int(c1), data);
    imageStore(i2D, ivec2(c2), data);
    imageStore(i3D, ivec3(c3), data);
    imageStore(i2DRect, ivec2(c2), data);
    imageStore(iCube, ivec3(c3), data);
    imageStore(iBuffer, int(c1), data);
    imageStore(i1DArray, ivec2(c2), data);
    imageStore(i2DArray, ivec3(c3), data);
    imageStore(iCubeArray, ivec3(c3), data);
    imageStore(i2DMS, ivec2(c2), 1, data);
    imageStore(i2DMSArray, ivec3(c3), 1, data);
}

vec4 testSparseTexture()
{
    vec4 texel = vec4(0.0);

    sparseTextureARB(s2D, c2, texel);
    sparseTextureARB(s3D, c3, texel);
    sparseTextureARB(sCube, c3, texel);
    sparseTextureARB(s2DShadow, c3, texel.x);
    sparseTextureARB(sCubeShadow, c4, texel.x);
    sparseTextureARB(s2DArray, c3, texel);
    sparseTextureARB(sCubeArray, c4, texel);
    sparseTextureARB(s2DArrayShadow, c4, texel.x);
    sparseTextureARB(s2DRect, c2, texel);
    sparseTextureARB(s2DRectShadow, c3, texel.x);
    sparseTextureARB(sCubeArrayShadow, c4, compare, texel.x);

    return texel;
}

vec4 testSparseTextureLod()
{
    vec4 texel = vec4(0.0);

    sparseTextureLodARB(s2D, c2, lod, texel);
    sparseTextureLodARB(s3D, c3, lod, texel);
    sparseTextureLodARB(sCube, c3, lod, texel);
    sparseTextureLodARB(s2DShadow, c3, lod, texel.x);
    sparseTextureLodARB(s2DArray, c3, lod, texel);
    sparseTextureLodARB(sCubeArray, c4, lod, texel);

    return texel;
}

vec4 testSparseTextureOffset()
{
    vec4 texel = vec4(0.0);

    sparseTextureOffsetARB(s2D, c2, offset2, texel);
    sparseTextureOffsetARB(s3D, c3, offset3, texel);
    sparseTextureOffsetARB(s2DRect, c2, offset2, texel);
    sparseTextureOffsetARB(s2DRectShadow, c3, offset2, texel.x);
    sparseTextureOffsetARB(s2DShadow, c3, offset2, texel.x);
    sparseTextureOffsetARB(s2DArray, c3, offset2, texel);
    sparseTextureOffsetARB(s2DArrayShadow, c4, offset2, texel.x);

    return texel;
}

vec4 testSparseTextureLodOffset()
{
    vec4 texel = vec4(0.0);

    sparseTextureLodOffsetARB(s2D, c2, lod, offset2, texel);
    sparseTextureLodOffsetARB(s3D, c3, lod, offset3, texel);
    sparseTextureLodOffsetARB(s2DShadow, c3, lod, offset2, texel.x);
    sparseTextureLodOffsetARB(s2DArray, c3, lod, offset2, texel);

    return texel;
}

vec4 testSparseTextureGrad()
{
    vec4 texel = vec4(0.0);

    sparseTextureGradARB(s2D, c2, dPdxy2, dPdxy2, texel);
    sparseTextureGradARB(s3D, c3, dPdxy3, dPdxy3, texel);
    sparseTextureGradARB(sCube, c3, dPdxy3, dPdxy3, texel);
    sparseTextureGradARB(s2DRect, c2, dPdxy2, dPdxy2, texel);
    sparseTextureGradARB(s2DRectShadow, c3, dPdxy2, dPdxy2, texel.x);
    sparseTextureGradARB(s2DShadow, c3, dPdxy2, dPdxy2, texel.x);
    sparseTextureGradARB(sCubeShadow, c4, dPdxy3, dPdxy3, texel.x);
    sparseTextureGradARB(s2DArray, c3, dPdxy2, dPdxy2, texel);
    sparseTextureGradARB(s2DArrayShadow, c4, dPdxy2, dPdxy2, texel.x);
    sparseTextureGradARB(sCubeArray, c4, dPdxy3, dPdxy3, texel);

    return texel;
}

vec4 testSparseTextureGradOffset()
{
    vec4 texel = vec4(0.0);

    sparseTextureGradOffsetARB(s2D, c2, dPdxy2, dPdxy2, offset2, texel);
    sparseTextureGradOffsetARB(s3D, c3, dPdxy3, dPdxy3, offset3, texel);
    sparseTextureGradOffsetARB(s2DRect, c2, dPdxy2, dPdxy2, offset2, texel);
    sparseTextureGradOffsetARB(s2DRectShadow, c3, dPdxy2, dPdxy2, offset2, texel.x);
    sparseTextureGradOffsetARB(s2DShadow, c3, dPdxy2, dPdxy2, offset2, texel.x);
    sparseTextureGradOffsetARB(s2DArray, c3, dPdxy2, dPdxy2, offset2, texel);
    sparseTextureGradOffsetARB(s2DArrayShadow, c4, dPdxy2, dPdxy2, offset2, texel.x);

    return texel;
}

vec4 testSparseTexelFetch()
{
    vec4 texel = vec4(0.0);

    sparseTexelFetchARB(s2D, ivec2(c2), int(lod), texel);
    sparseTexelFetchARB(s3D, ivec3(c3), int(lod), texel);
    sparseTexelFetchARB(s2DRect, ivec2(c2), texel);
    sparseTexelFetchARB(s2DArray, ivec3(c3), int(lod), texel);
    sparseTexelFetchARB(s2DMS, ivec2(c2), 1, texel);
    sparseTexelFetchARB(s2DMSArray, ivec3(c3), 2, texel);

    return texel;
}

vec4 testSparseTexelFetchOffset()
{
    vec4 texel = vec4(0.0);

    sparseTexelFetchOffsetARB(s2D, ivec2(c2), int(lod), offset2, texel);
    sparseTexelFetchOffsetARB(s3D, ivec3(c3), int(lod), offset3, texel);
    sparseTexelFetchOffsetARB(s2DRect, ivec2(c2), offset2, texel);
    sparseTexelFetchOffsetARB(s2DArray, ivec3(c3), int(lod), offset2, texel);

    return texel;
}

vec4 testSparseTextureGather()
{
    vec4 texel = vec4(0.0);

    sparseTextureGatherARB(s2D, c2, texel, 0);
    sparseTextureGatherARB(s2DArray, c3, texel, 0);
    sparseTextureGatherARB(sCube, c3, texel, 0);
    sparseTextureGatherARB(sCubeArray, c4, texel, 0);
    sparseTextureGatherARB(s2DRect, c2, texel, 0);
    sparseTextureGatherARB(s2DShadow, c2, compare, texel);
    sparseTextureGatherARB(s2DArrayShadow, c3, compare, texel);
    sparseTextureGatherARB(sCubeShadow, c3, compare, texel);
    sparseTextureGatherARB(sCubeArrayShadow, c4, compare, texel);
    sparseTextureGatherARB(s2DRectShadow, c2, compare, texel);

    return texel;
}

vec4 testSparseTextureGatherOffset()
{
    vec4 texel = vec4(0.0);

    sparseTextureGatherOffsetARB(s2D, c2, offset2, texel, 0);
    sparseTextureGatherOffsetARB(s2DArray, c3, offset2, texel, 0);
    sparseTextureGatherOffsetARB(s2DRect, c2, offset2, texel, 0);
    sparseTextureGatherOffsetARB(s2DShadow, c2, compare, offset2, texel);
    sparseTextureGatherOffsetARB(s2DArrayShadow, c3, compare, offset2, texel);
    sparseTextureGatherOffsetARB(s2DRectShadow, c2, compare, offset2, texel);

    return texel;
}

vec4 testSparseTextureGatherOffsets()
{
    vec4 texel = vec4(0.0);
    const ivec2 constOffsets[4] = ivec2[4](ivec2(1,2), ivec2(3,4), ivec2(15,16), ivec2(-2,0));

    sparseTextureGatherOffsetsARB(s2D, c2, constOffsets, texel, 0);
    sparseTextureGatherOffsetsARB(s2DArray, c3, constOffsets, texel, 0);
    sparseTextureGatherOffsetsARB(s2DRect, c2, constOffsets, texel, 0);
    sparseTextureGatherOffsetsARB(s2DShadow, c2, compare, constOffsets, texel);
    sparseTextureGatherOffsetsARB(s2DArrayShadow, c3, compare, constOffsets, texel);
    sparseTextureGatherOffsetsARB(s2DRectShadow, c2, compare, constOffsets, texel);

    return texel;
}

vec4 testSparseTextureGatherLod()
{
    vec4 texel = vec4(0.0);

    sparseTextureGatherLodAMD(s2D, c2, lod, texel, 0);
    sparseTextureGatherLodAMD(s2DArray, c3, lod, texel, 0);
    sparseTextureGatherLodAMD(sCube, c3, lod, texel, 0);
    sparseTextureGatherLodAMD(sCubeArray, c4, lod, texel, 0);

    return texel;
}

vec4 testSparseTextureGatherLodOffset()
{
    vec4 texel = vec4(0.0);

    sparseTextureGatherLodOffsetAMD(s2D, c2, lod, offset2, texel, 0);
    sparseTextureGatherLodOffsetAMD(s2DArray, c3, lod, offset2, texel, 0);

    return texel;
}

vec4 testSparseTextureGatherLodOffsets()
{
    vec4 texel = vec4(0.0);

    sparseTextureGatherLodOffsetsAMD(s2D, c2, lod, offsets, texel, 0);
    sparseTextureGatherLodOffsetsAMD(s2DArray, c3, lod, offsets, texel, 0);

    return texel;
}

vec4 testSparseImageLoad()
{
    vec4 texel = vec4(0.0);

    sparseImageLoadARB(i2D, ivec2(c2), texel);
    sparseImageLoadARB(i3D, ivec3(c3), texel);
    sparseImageLoadARB(i2DRect, ivec2(c2), texel);
    sparseImageLoadARB(iCube, ivec3(c3), texel);
    sparseImageLoadARB(i2DArray, ivec3(c3), texel);
    sparseImageLoadARB(iCubeArray, ivec3(c3), texel);
    sparseImageLoadARB(i2DMS, ivec2(c2), 1, texel);
    sparseImageLoadARB(i2DMSArray, ivec3(c3), 2, texel);

    return texel;
}

vec4 testSparseTextureClamp()
{
    vec4 texel = vec4(0.0);

    sparseTextureClampARB(s2D, c2, lodClamp, texel);
    sparseTextureClampARB(s3D, c3, lodClamp, texel);
    sparseTextureClampARB(sCube, c3, lodClamp, texel);
    sparseTextureClampARB(s2DShadow, c3, lodClamp, texel.x);
    sparseTextureClampARB(sCubeShadow, c4, lodClamp, texel.x);
    sparseTextureClampARB(s2DArray, c3, lodClamp, texel);
    sparseTextureClampARB(sCubeArray, c4, lodClamp, texel);
    sparseTextureClampARB(s2DArrayShadow, c4, lodClamp, texel.x);
    sparseTextureClampARB(sCubeArrayShadow, c4, compare, lodClamp, texel.x);

    return texel;
}

vec4 testTextureClamp()
{
    vec4 texel = vec4(0.0);

    texel   += textureClampARB(s1D, c1, lodClamp);
    texel   += textureClampARB(s2D, c2, lodClamp);
    texel   += textureClampARB(s3D, c3, lodClamp);
    texel   += textureClampARB(sCube, c3, lodClamp);
    texel.x += textureClampARB(s1DShadow, c3, lodClamp);
    texel.x += textureClampARB(s2DShadow, c3, lodClamp);
    texel.x += textureClampARB(sCubeShadow, c4, lodClamp);
    texel   += textureClampARB(s1DArray, c2, lodClamp);
    texel   += textureClampARB(s2DArray, c3, lodClamp);
    texel   += textureClampARB(sCubeArray, c4, lodClamp);
    texel.x += textureClampARB(s1DArrayShadow, c3, lodClamp);
    texel.x += textureClampARB(s2DArrayShadow, c4, lodClamp);
    texel.x += textureClampARB(sCubeArrayShadow, c4, compare, lodClamp);

    return texel;
}

vec4 testSparseTextureOffsetClamp()
{
    vec4 texel = vec4(0.0);

    sparseTextureOffsetClampARB(s2D, c2, offset2, lodClamp, texel);
    sparseTextureOffsetClampARB(s3D, c3, offset3, lodClamp, texel);
    sparseTextureOffsetClampARB(s2DShadow, c3, offset2, lodClamp, texel.x);
    sparseTextureOffsetClampARB(s2DArray, c3, offset2, lodClamp, texel);
    sparseTextureOffsetClampARB(s2DArrayShadow, c4, offset2, lodClamp, texel.x);

    return texel;
}

vec4 testTextureOffsetClamp()
{
    vec4 texel = vec4(0.0);

    texel   += textureOffsetClampARB(s1D, c1, offset1, lodClamp);
    texel   += textureOffsetClampARB(s2D, c2, offset2, lodClamp);
    texel   += textureOffsetClampARB(s3D, c3, offset3, lodClamp);
    texel.x += textureOffsetClampARB(s1DShadow, c3, offset1, lodClamp);
    texel.x += textureOffsetClampARB(s2DShadow, c3, offset2, lodClamp);
    texel   += textureOffsetClampARB(s1DArray, c2, offset1, lodClamp);
    texel   += textureOffsetClampARB(s2DArray, c3, offset2, lodClamp);
    texel.x += textureOffsetClampARB(s1DArrayShadow, c3, offset1, lodClamp);
    texel.x += textureOffsetClampARB(s2DArrayShadow, c4, offset2, lodClamp);
    
    return texel;
}

vec4 testSparseTextureGradClamp()
{
    vec4 texel = vec4(0.0);

    sparseTextureGradClampARB(s2D, c2, dPdxy2, dPdxy2, lodClamp, texel);
    sparseTextureGradClampARB(s3D, c3, dPdxy3, dPdxy3, lodClamp, texel);
    sparseTextureGradClampARB(sCube, c3, dPdxy3, dPdxy3, lodClamp, texel);
    sparseTextureGradClampARB(s2DShadow, c3, dPdxy2, dPdxy2, lodClamp, texel.x);
    sparseTextureGradClampARB(sCubeShadow, c4, dPdxy3, dPdxy3, lodClamp, texel.x);
    sparseTextureGradClampARB(s2DArray, c3, dPdxy2, dPdxy2, lodClamp, texel);
    sparseTextureGradClampARB(s2DArrayShadow, c4, dPdxy2, dPdxy2, lodClamp, texel.x);
    sparseTextureGradClampARB(sCubeArray, c4, dPdxy3, dPdxy3, lodClamp, texel);

    return texel;
}

vec4 testTextureGradClamp()
{
    vec4 texel = vec4(0.0);

    texel   += textureGradClampARB(s1D, c1, dPdxy1, dPdxy1, lodClamp);
    texel   += textureGradClampARB(s2D, c2, dPdxy2, dPdxy2, lodClamp);
    texel   += textureGradClampARB(s3D, c3, dPdxy3, dPdxy3, lodClamp);
    texel   += textureGradClampARB(sCube, c3, dPdxy3, dPdxy3, lodClamp);
    texel.x += textureGradClampARB(s1DShadow, c3, dPdxy1, dPdxy1, lodClamp);
    texel.x += textureGradClampARB(s2DShadow, c3, dPdxy2, dPdxy2, lodClamp);
    texel.x += textureGradClampARB(sCubeShadow, c4, dPdxy3, dPdxy3, lodClamp);
    texel   += textureGradClampARB(s1DArray, c2, dPdxy1, dPdxy1, lodClamp);
    texel   += textureGradClampARB(s2DArray, c3, dPdxy2, dPdxy2, lodClamp);
    texel.x += textureGradClampARB(s1DArrayShadow, c3, dPdxy1, dPdxy1, lodClamp);
    texel.x += textureGradClampARB(s2DArrayShadow, c4, dPdxy2, dPdxy2, lodClamp);
    texel   += textureGradClampARB(sCubeArray, c4, dPdxy3, dPdxy3, lodClamp);

    return texel;
}

vec4 testSparseTextureGradOffsetClamp()
{
    vec4 texel = vec4(0.0);

    sparseTextureGradOffsetClampARB(s2D, c2, dPdxy2, dPdxy2, offset2, lodClamp, texel);
    sparseTextureGradOffsetClampARB(s3D, c3, dPdxy3, dPdxy3, offset3, lodClamp, texel);
    sparseTextureGradOffsetClampARB(s2DShadow, c3, dPdxy2, dPdxy2, offset2, lodClamp, texel.x);
    sparseTextureGradOffsetClampARB(s2DArray, c3, dPdxy2, dPdxy2, offset2, lodClamp, texel);
    sparseTextureGradOffsetClampARB(s2DArrayShadow, c4, dPdxy2, dPdxy2, offset2, lodClamp, texel.x);

    return texel;
}

vec4 testTextureGradOffsetClamp()
{
    vec4 texel = vec4(0.0);

    texel   += textureGradOffsetClampARB(s1D, c1, dPdxy1, dPdxy1, offset1, lodClamp);
    texel   += textureGradOffsetClampARB(s2D, c2, dPdxy2, dPdxy2, offset2, lodClamp);
    texel   += textureGradOffsetClampARB(s3D, c3, dPdxy3, dPdxy3, offset3, lodClamp);
    texel.x += textureGradOffsetClampARB(s1DShadow, c3, dPdxy1, dPdxy1, offset1, lodClamp);
    texel.x += textureGradOffsetClampARB(s2DShadow, c3, dPdxy2, dPdxy2, offset2, lodClamp);
    texel   += textureGradOffsetClampARB(s1DArray, c2, dPdxy1, dPdxy1, offset1, lodClamp);
    texel   += textureGradOffsetClampARB(s2DArray, c3, dPdxy2, dPdxy2, offset2, lodClamp);
    texel.x += textureGradOffsetClampARB(s1DArrayShadow, c3, dPdxy1, dPdxy1, offset1, lodClamp);
    texel.x += textureGradOffsetClampARB(s2DArrayShadow, c4, dPdxy2, dPdxy2, offset2, lodClamp);

    return texel;
}

vec4 testCombinedTextureSampler()
{
    vec4 texel = vec4(0.0);

    texel   += texture(sampler1D(t1D, s), c1);
    texel   += texture(sampler2D(t2D, s), c2);
    texel   += texture(sampler3D(t3D, s), c3);
    texel   += texture(samplerCube(tCube, s), c3);
    texel.x += texture(sampler1DShadow(t1D, sShadow), c3);
    texel.x += texture(sampler2DShadow(t2D, sShadow), c3);
    texel.x += texture(samplerCubeShadow(tCube, sShadow), c4);
    texel   += texture(sampler1DArray(t1DArray, s), c2);
    texel   += texture(sampler2DArray(t2DArray, s), c3);
    texel   += texture(samplerCubeArray(tCubeArray, s), c4);
    texel.x += texture(sampler1DArrayShadow(t1DArray, sShadow), c3);
    texel.x += texture(sampler2DArrayShadow(t2DArray, sShadow), c4);
    texel   += texture(sampler2DRect(t2DRect, s), c2);
    texel.x += texture(sampler2DRectShadow(t2DRect, sShadow), c3);
    texel.x += texture(samplerCubeArrayShadow(tCubeArray, sShadow), c4, compare);

    return texel;
}

vec4 testSubpassLoad()
{
    return subpassLoad(subpass) + subpassLoad(subpassMS, 2);
}

void main()
{
    vec4 result = vec4(0.0);

    result  += testTexture();
    result  += testTextureProj();
    result  += testTextureLod();
    result  += testTextureOffset();
    result  += testTextureLodOffset();
    result  += testTextureProjLodOffset();
    result  += testTexelFetch();
    result  += testTexelFetchOffset();
    result  += testTextureGrad();
    result  += testTextureGradOffset();
    result  += testTextureProjGrad();
    result  += testTextureProjGradoffset();
    result  += testTextureGather();
    result  += testTextureGatherOffset();
    result  += testTextureGatherOffsets();
    result  += testTextureGatherLod();
    result  += testTextureGatherLodOffset();
    result  += testTextureGatherLodOffsets();

    result    += vec4(testTextureSize());
    result.xy += vec2(testTextureQueryLod());
    result.x  += testTextureQueryLevels();
    result.x  += testTextureSamples();

    result  += testImageLoad();
    testImageStore(result);

    result += testSparseTexture();
    result += testSparseTextureLod();
    result += testSparseTextureOffset();
    result += testSparseTextureLodOffset();
    result += testSparseTextureGrad();
    result += testSparseTextureGradOffset();
    result += testSparseTexelFetch();
    result += testSparseTexelFetchOffset();
    result += testSparseTextureGather();
    result += testSparseTextureGatherOffset();
    result += testSparseTextureGatherOffsets();
    result += testSparseTextureGatherLod();
    result += testSparseTextureGatherLodOffset();
    result += testSparseTextureGatherLodOffsets();

    result += testSparseImageLoad();

    result += testSparseTextureClamp();
    result += testTextureClamp();
    result += testSparseTextureOffsetClamp();
    result += testTextureOffsetClamp();
    result += testSparseTextureGrad();
    result += testTextureGrad();
    result += testSparseTextureGradOffsetClamp();
    result += testTextureGradOffsetClamp();

    result += testCombinedTextureSampler();
    result += testSubpassLoad();

    fragColor = result;
}

