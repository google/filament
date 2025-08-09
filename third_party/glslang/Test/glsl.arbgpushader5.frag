#version 150
#extension GL_ARB_gpu_shader5 : enable

// Testing storage qualifiers sample in/ sample out
sample in vec4 colorSampIn;
sample out vec4 colorSampOut; // error: sampler out is not allowed in fragment shader

// Testing integer expression for indexing
#define SAMPLER_ARRAY_SIZE 10
uniform sampler2D tex[SAMPLER_ARRAY_SIZE];
vec2 coord;

void testIndexing()
{
    // Enable gl_SampleMaskIn built in variable in fragment shader
    int var = gl_SampleMaskIn[0];
    int i = 0;
    texture(tex[i], coord); //integer expression for indexing

}

// 1. Testing implicity conversin int -> uint
// 2. Test function overloading with implicit conversion

flat in int b;

// Test function overloading with implicit conversion
void func(uint a, uvec4 b)
{

}

void testImplicitConv()
{
    //precise vec2 h;
    //Test implicit conversion
    uint uv = int(0); // int -> uint
    uvec2 uv2 = ivec2 (0); // ivec2 -> uvec2
    uvec3 uv3 = ivec3 (0); // ivec3 -> uvec3
    uvec4 uv4 = ivec4 (0); // ivec4 -> uvec4

    // Function overloading with implict conversion int->uint
    func(b, ivec4(1));        
}


// Test ARB_gpu_shader5 builtins
uniform sampler2DShadow var16;
uniform samplerCubeShadow var17;
uniform sampler2DArrayShadow var18;
uniform samplerCubeArrayShadow var19;
uniform sampler2DRectShadow var20;
uniform samplerCubeArray svar21;
uniform usamplerCubeArray svar22;
uniform isamplerCubeArray svar23;
uniform sampler2D svar24;
uniform usampler2D svar25;
uniform isampler2D svar26;
uniform sampler2DRect svar27;
uniform usampler2DRect svar28;
uniform isampler2DRect svar29;
uniform samplerCube svar30;
uniform usamplerCube svar31;
uniform isamplerCube svar32;
uniform sampler2DArray svar33;
uniform usampler2DArray svar34;
uniform isampler2DArray svar35;
in float fvar0;
in vec2 fvar1;
in vec3 fvar2;
in vec4 fvar3;


void testBuiltins() {
float var0;
vec2 var1;
vec3 var2;
vec4 var3;
int var4;
ivec2 var5;
ivec3 var6;
ivec4 var7;
uint var8;
uvec2 var9;
uvec3 var10;
uvec4 var11;
bool var12;
bvec2 var13;
bvec3 var14;
bvec4 var15;
const ivec2 constOffsets[4] = ivec2[4](ivec2(1,2), ivec2(3,4), ivec2(15,16), ivec2(-2,0));

// Function Signature : genType fma(genType, genType, genType)
var0 = fma(var0, var0, var0);
var1 = fma(var1, var1, var1);
var2 = fma(var2, var2, var2);
var3 = fma(var3, var3, var3);
// Function Signature : genType frexp(genType, genIType)
var0 = frexp(var0, var4);
var1 = frexp(var1, var5);
var2 = frexp(var2, var6);
var3 = frexp(var3, var7);

// Function Signature : genType ldexp(genType, genIType)
var0 = ldexp(var0, var4);
var1 = ldexp(var1, var5);
var2 = ldexp(var2, var6);
var3 = ldexp(var3, var7);

// Function Signature : genIType bitfieldExtract(genIType, int, int)
var4 = bitfieldExtract(var4, var4, var4);
var5 = bitfieldExtract(var5, var4, var4);
var6 = bitfieldExtract(var6, var4, var4);
var7 = bitfieldExtract(var7, var4, var4);
// Function Signature : genUType bitfieldExtract(genUType, int, int)
var8 = bitfieldExtract(var8, var4, var4);
var9 = bitfieldExtract(var9, var4, var4);
var10 = bitfieldExtract(var10, var4, var4);
var11 = bitfieldExtract(var11, var4, var4);
// Function Signature : genIType bitfieldInsert(genIType, genIType, int, int)
var4 = bitfieldInsert(var4, var4, var4, var4);
var5 = bitfieldInsert(var5, var5, var4, var4);
var6 = bitfieldInsert(var6, var6, var4, var4);
var7 = bitfieldInsert(var7, var7, var4, var4);
// Function Signature : genUType bitfieldInsert(genUType, genUType, int, int)
var8 = bitfieldInsert(var8, var8, var4, var4);
var9 = bitfieldInsert(var9, var9, var4, var4);
var10 = bitfieldInsert(var10, var10, var4, var4);
var11 = bitfieldInsert(var11, var11, var4, var4);
// Function Signature : genIType bitfieldReverse(genIType)
var4 = bitfieldReverse(var4);
var5 = bitfieldReverse(var5);
var6 = bitfieldReverse(var6);
var7 = bitfieldReverse(var7);
// Function Signature : genUType bitfieldReverse(genUType)
var8 = bitfieldReverse(var8);
var9 = bitfieldReverse(var9);
var10 = bitfieldReverse(var10);
var11 = bitfieldReverse(var11);
// Function Signature : genIType bitCount(genIType)
var4 = bitCount(var4);
var5 = bitCount(var5);
var6 = bitCount(var6);
var7 = bitCount(var7);
// Function Signature : genUType bitCount(genUType)
var8 = bitCount(var8);
var9 = bitCount(var9);
var10 = bitCount(var10);
var11 = bitCount(var11);
// Function Signature : genUType findMSB(genUType)
var8 = findMSB(var8);
var9 = findMSB(var9);
var10 = findMSB(var10);
var11 = findMSB(var11);
// Function Signature : genIType findLSB(genIType)
var4 = findMSB(var4);
var5 = findMSB(var5);
var6 = findMSB(var6);
var7 = findMSB(var7);
// Function Signature : genIType findLSB(genIType)
var4 = findLSB(var4);
var5 = findLSB(var5);
var6 = findLSB(var6);
var7 = findLSB(var7);
// Function Signature : genUType findLSB(genUType)
var8 = findLSB(var8);
var9 = findLSB(var9);
var10 = findLSB(var10);
var11 = findLSB(var11);

// Function Signature : genIType floatBitsToInt(genType)
var4 = floatBitsToInt(var0);
var5 = floatBitsToInt(var1);
var6 = floatBitsToInt(var2);
var7 = floatBitsToInt(var3);
// Function Signature : genUType floatBitsToUint(genType)
var8 = floatBitsToUint(var0);
var9 = floatBitsToUint(var1);
var10 = floatBitsToUint(var2);
var11 = floatBitsToUint(var3);
// Function Signature : genType intBitsToFloat(genIType)
var0 = intBitsToFloat(var4);
var1 = intBitsToFloat(var5);
var2 = intBitsToFloat(var6);
var3 = intBitsToFloat(var7);
// Function Signature : genType uintBitsToFloat(genUType)
var0 = uintBitsToFloat(var8);
var1 = uintBitsToFloat(var9);
var2 = uintBitsToFloat(var10);
var3 = uintBitsToFloat(var11);

// Function Signature : genUType uaddCarry(genUType, genUType, genUType)
var8 = uaddCarry(var8, var8, var8);
var9 = uaddCarry(var9, var9, var9);
var10 = uaddCarry(var10, var10, var10);
var11 = uaddCarry(var11, var11, var11);
// Function Signature : genUType usubBorrow(genUType, genUType, genUType)
var8 = usubBorrow(var8, var8, var8);
var9 = usubBorrow(var9, var9, var9);
var10 = usubBorrow(var10, var10, var10);
var11 = usubBorrow(var11, var11, var11);
// Function Signature : void umulExtended(genUType, genUType, genUType, genUType)
umulExtended(var8, var8, var8, var8);
umulExtended(var9, var9, var9, var9);
umulExtended(var10, var10, var10, var10);
umulExtended(var11, var11, var11, var11);
// Function Signature : void imulExtended(genIType, genIType, genIType, genIType)
imulExtended(var4, var4, var4, var4);
imulExtended(var5, var5, var5, var5);
imulExtended(var6, var6, var6, var6);
imulExtended(var7, var7, var7, var7);
// Generate specific builtins
var8 = packUnorm2x16(var1);
var8 = packUnorm4x8(var3);
var8 = packSnorm4x8(var3);

var1 = unpackUnorm2x16(var8);
var3 = unpackUnorm4x8(var8);
var3 = unpackSnorm4x8(var8);

var0 = interpolateAtCentroid(fvar0);
var1 = interpolateAtCentroid(fvar1);
var2 = interpolateAtCentroid(fvar2);
var3 = interpolateAtCentroid(fvar3);
var0 = interpolateAtSample(fvar0, var4);
var1 = interpolateAtSample(fvar1, var4);
var2 = interpolateAtSample(fvar2, var4);
var3 = interpolateAtSample(fvar3, var4);
var0 = interpolateAtOffset(fvar0, var1);
var1 = interpolateAtOffset(fvar1, var1);
var2 = interpolateAtOffset(fvar2, var1);
var3 = interpolateAtOffset(fvar3, var1);

// Generate textue specific intrinsics
// Function Signature : gvec4 textureGatherOffset(gsampler2D, vec2, ivec2)
var3 = textureGatherOffset(svar24, var1, var5);
var11 = textureGatherOffset(svar25, var1, var5);
var7 = textureGatherOffset(svar26, var1, var5);

// Function Signature : gvec4 textureGatherOffset(gsampler2DArray, vec3, ivec2)
var3 = textureGatherOffset(svar33, var2, var5);
var11 = textureGatherOffset(svar34, var2, var5);
var7 = textureGatherOffset(svar35, var2, var5);
// Function Signature : gvec4 textureGatherOffset(gsampler2DRect, vec2, ivec2)
var3 = textureGatherOffset(svar27, var1, var5);
var11 = textureGatherOffset(svar28, var1, var5);
var7 = textureGatherOffset(svar29, var1, var5);
// Function Signature : vec4 textureGatherOffset(sampler2DShadow, vec2, float, ivec2)
var3 = textureGatherOffset(var16, var1, var0, var5);
// Function Signature : vec4 textureGatherOffset(sampler2DArrayShadow, vec3, float, ivec2)
var3 = textureGatherOffset(var18, var2, var0, var5);
// Function Signature : vec4 textureGatherOffset(sampler2DRectShadow, vec2, float, ivec2)
var3 = textureGatherOffset(var20, var1, var0, var5);

// Function Signature : gvec4 textureGather(gsampler2D, vec2)
var3 = textureGather(svar24, var1);
var11 = textureGather(svar25, var1);
var7 = textureGather(svar26, var1);
// Function Signature : gvec4 textureGather(gsampler2DArray, vec3)
var3 = textureGather(svar33, var2);
var11 = textureGather(svar34, var2);
var7 = textureGather(svar35, var2);
// Function Signature : gvec4 textureGather(gsamplerCube, vec3)
var3 = textureGather(svar30, var2);
var11 = textureGather(svar31, var2);
var7 = textureGather(svar32, var2);
// Function Signature : gvec4 textureGather(gsamplerCubeArray, vec4)
var3 = textureGather(svar21, var3);
var11 = textureGather(svar22, var3);
var7 = textureGather(svar23, var3);
// Function Signature : gvec4 textureGather(gsampler2DRect, vec4)
var3 = textureGather(svar27, var1);
var11 = textureGather(svar28, var1);
var7 = textureGather(svar29, var1);
// Function Signature : vec4 textureGather(sampler2DShadow, vec2, float)
var3 = textureGather(var16, var1, var0);
// Function Signature : vec4 textureGather(sampler2DArrayShadow, vec3, float)
var3 = textureGather(var18, var2, var0);
// Function Signature : vec4 textureGather(samplerCubeShadow, vec3, float)
var3 = textureGather(var17, var2, var0);
// Function Signature : vec4 textureGather(samplerCubeArrayShadow, vec4, float)
var3 = textureGather(var19, var3, var0);
// Function Signature : vec4 textureGather(sampler2DRectShadow, vec2, float)
var3 = textureGather(var20, var1, var0);

// Function Signature : gvec4 textureGatherOffsets(gsampler2D, vec2, ivec2[4])
var3 = textureGatherOffsets(svar24, var1, constOffsets);
var11 = textureGatherOffsets(svar25, var1, constOffsets);
var7 = textureGatherOffsets(svar26, var1, constOffsets);
// Function Signature : gvec4 textureGatherOffsets(gsampler2DArray, vec2, ivec2[4])
var3 = textureGatherOffsets(svar33, var2, constOffsets);
var11 = textureGatherOffsets(svar34, var2, constOffsets);
var7 = textureGatherOffsets(svar35, var2, constOffsets);
// Function Signature : gvec4 textureGatherOffsets(gsampler2DRect, vec2, ivec2[4])
var3 = textureGatherOffsets(svar27, var1, constOffsets);
var11 = textureGatherOffsets(svar28, var1, constOffsets);
var7 = textureGatherOffsets(svar29, var1, constOffsets);
// Function Signature : vec4 textureGatherOffsets(sampler2DShadow, vec2, float, ivec2[4])
var3 = textureGatherOffsets(var16, var1, var0, constOffsets);
// Function Signature : vec4 textureGatherOffsets(sampler2DArrayShadow, vec3, float, ivec2[4])
var3 = textureGatherOffsets(var18, var2, var0, constOffsets);
// Function Signature : vec4 textureGatherOffsets(sampler2DRectShadow, vec2, float, ivec2[4])
var3 = textureGatherOffsets(var20, var1, var0, constOffsets);
}


// Testing integer expression for indexing in interface blocks
uniform ubName { 
	int i; 
} ubInst[4];


void testBlockIndexing()
{
    ubInst[1];
    //bbInst[2];
    int i = 0;
    ubInst[i]; //dynamic indexing via ARB_gpu_shader5 or enabled since version 430
}