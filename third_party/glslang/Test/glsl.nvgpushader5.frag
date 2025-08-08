#version 150
#extension GL_NV_gpu_shader5 : enable
#extension GL_ARB_gpu_shader_fp64 : enable

// Testing integer expression for indexing
#define SAMPLER_ARRAY_SIZE 10
uniform sampler2D tex[SAMPLER_ARRAY_SIZE];
// Testing integer expression for indexing in interface blocks
uniform ubName { 
	int i; 
} ubInst[4];


vec2 coord;
void testIndexing()
{
    int i = 0;
    texture(tex[i], coord); //integer expression for indexing
    ubInst[i]; //dynamic indexing via ARB_gpu_shader5 or enabled since version 430

}


void testImplictConversion() {
// int ====> uint, int64_t, uint64_t, float, double
uint var0 = int (0); // int -> uint
int64_t var1 = int (0); // int -> int64_t
uint64_t var2 = int (0); // int -> uint64_t
float var3 = int (0); // int -> float
double var4 = int (0); // int -> double


// ivec2 ====> uvec2, i64vec2, u64vec2, vec2, dvec2
uvec2 var5 = ivec2 (0); // ivec2 -> uvec2
i64vec2 var6 = ivec2 (0); // ivec2 -> i64vec2
u64vec2 var7 = ivec2 (0); // ivec2 -> u64vec2
vec2 var8 = ivec2 (0); // ivec2 -> vec2
dvec2 var9 = ivec2 (0); // ivec2 -> dvec2


// ivec3 ====> uvec3, i64vec3, u64vec3, vec3, dvec3
uvec3 var10 = ivec3 (0); // ivec3 -> uvec3
i64vec3 var11 = ivec3 (0); // ivec3 -> i64vec3
u64vec3 var12 = ivec3 (0); // ivec3 -> u64vec3
vec3 var13 = ivec3 (0); // ivec3 -> vec3
dvec3 var14 = ivec3 (0); // ivec3 -> dvec3


// ivec4 ====> uvec4, i64vec4, u64vec4, vec4, dvec4
uvec4 var15 = ivec4 (0); // ivec4 -> uvec4
i64vec4 var16 = ivec4 (0); // ivec4 -> i64vec4
u64vec4 var17 = ivec4 (0); // ivec4 -> u64vec4
vec4 var18 = ivec4 (0); // ivec4 -> vec4
dvec4 var19 = ivec4 (0); // ivec4 -> dvec4


// int8_t ====> int, int64_t, uint, uint64_t, float, double
int var20 = int8_t (0); // int8_t -> int
int64_t var21 = int8_t (0); // int8_t -> int64_t
uint var22 = int8_t (0); // int8_t -> uint
uint64_t var23 = int8_t (0); // int8_t -> uint64_t
float var24 = int8_t (0); // int8_t -> float
double var25 = int8_t (0); // int8_t -> double


// int16_t ====> int, int64_t, uint, uint64_t, float, double
int var26 = int16_t (0); // int16_t -> int
int64_t var27 = int16_t (0); // int16_t -> int64_t
uint var28 = int16_t (0); // int16_t -> uint
uint64_t var29 = int16_t (0); // int16_t -> uint64_t
float var30 = int16_t (0); // int16_t -> float
double var31 = int16_t (0); // int16_t -> double


// i8vec2 ====> ivec2, i64vec2, uvec2, u64vec2, vec2, dvec2
ivec2 var32 = i8vec2 (0); // i8vec2 -> ivec2
i64vec2 var33 = i8vec2 (0); // i8vec2 -> i64vec2
uvec2 var34 = i8vec2 (0); // i8vec2 -> uvec2
u64vec2 var35 = i8vec2 (0); // i8vec2 -> u64vec2
vec2 var36 = i8vec2 (0); // i8vec2 -> vec2
dvec2 var37 = i8vec2 (0); // i8vec2 -> dvec2


// i16vec2 ====> ivec2, i64vec2, uvec2, u64vec2, vec2, dvec2
ivec2 var38 = i16vec2 (0); // i16vec2 -> ivec2
i64vec2 var39 = i16vec2 (0); // i16vec2 -> i64vec2
uvec2 var40 = i16vec2 (0); // i16vec2 -> uvec2
u64vec2 var41 = i16vec2 (0); // i16vec2 -> u64vec2
vec2 var42 = i16vec2 (0); // i16vec2 -> vec2
dvec2 var43 = i16vec2 (0); // i16vec2 -> dvec2


// i8vec3 ====> ivec3, i64vec3, uvec3, u64vec3, vec3, dvec3
ivec3 var44 = i8vec3 (0); // i8vec3 -> ivec3
i64vec3 var45 = i8vec3 (0); // i8vec3 -> i64vec3
uvec3 var46 = i8vec3 (0); // i8vec3 -> uvec3
u64vec3 var47 = i8vec3 (0); // i8vec3 -> u64vec3
vec3 var48 = i8vec3 (0); // i8vec3 -> vec3
dvec3 var49 = i8vec3 (0); // i8vec3 -> dvec3


// i16vec3 ====> uvec3, u64vec3, vec3, dvec3
uvec3 var50 = i16vec3 (0); // i16vec3 -> uvec3
u64vec3 var51 = i16vec3 (0); // i16vec3 -> u64vec3
vec3 var52 = i16vec3 (0); // i16vec3 -> vec3
dvec3 var53 = i16vec3 (0); // i16vec3 -> dvec3


// i8vec4 ====> ivec4, i64vec4, uvec4, u64vec4, vec4, dvec4
ivec4 var54 = i8vec4 (0); // i8vec4 -> ivec4
i64vec4 var55 = i8vec4 (0); // i8vec4 -> i64vec4
uvec4 var56 = i8vec4 (0); // i8vec4 -> uvec4
u64vec4 var57 = i8vec4 (0); // i8vec4 -> u64vec4
vec4 var58 = i8vec4 (0); // i8vec4 -> vec4
dvec4 var59 = i8vec4 (0); // i8vec4 -> dvec4


// i16vec4 ====> uvec4, u64vec4, vec4, dvec4
uvec4 var60 = i16vec4 (0); // i16vec4 -> uvec4
u64vec4 var61 = i16vec4 (0); // i16vec4 -> u64vec4
vec4 var62 = i16vec4 (0); // i16vec4 -> vec4
dvec4 var63 = i16vec4 (0); // i16vec4 -> dvec4


// int64_t ====> uint64_t, double
uint64_t var64 = int64_t (0); // int64_t -> uint64_t
double var65 = int64_t (0); // int64_t -> double


// i64vec2 ====> u64vec2, dvec2
u64vec2 var66 = i64vec2 (0); // i64vec2 -> u64vec2
dvec2 var67 = i64vec2 (0); // i64vec2 -> dvec2


// i64vec3 ====> u64vec3, dvec3
u64vec3 var68 = i64vec3 (0); // i64vec3 -> u64vec3
dvec3 var69 = i64vec3 (0); // i64vec3 -> dvec3


// i64vec4 ====> u64vec4, dvec4
u64vec4 var70 = i64vec4 (0); // i64vec4 -> u64vec4
dvec4 var71 = i64vec4 (0); // i64vec4 -> dvec4


// uint ====> uint64_t, float, double
uint64_t var72 = uint (0); // uint -> uint64_t
float var73 = uint (0); // uint -> float
double var74 = uint (0); // uint -> double


// uvec2 ====> u64vec2, vec2, dvec2
u64vec2 var75 = uvec2 (0); // uvec2 -> u64vec2
vec2 var76 = uvec2 (0); // uvec2 -> vec2
dvec2 var77 = uvec2 (0); // uvec2 -> dvec2


// uvec3 ====> u64vec3, vec3, dvec3
u64vec3 var78 = uvec3 (0); // uvec3 -> u64vec3
vec3 var79 = uvec3 (0); // uvec3 -> vec3
dvec3 var80 = uvec3 (0); // uvec3 -> dvec3


// uvec4 ====> u64vec4, vec4, dvec4
u64vec4 var81 = uvec4 (0); // uvec4 -> u64vec4
vec4 var82 = uvec4 (0); // uvec4 -> vec4
dvec4 var83 = uvec4 (0); // uvec4 -> dvec4

// uint8_t ====> uint, uint64_t, float, double
uint var84 = uint8_t (0); // uint8_t -> uint
uint64_t var85 = uint8_t (0); // uint8_t -> uint64_t
float var86 = uint8_t (0); // uint8_t -> float
double var87 = uint8_t (0); // uint8_t -> double


// uint16_t ====> uint, uint64_t, float, double
uint var88 = uint16_t (0); // uint16_t -> uint
uint64_t var89 = uint16_t (0); // uint16_t -> uint64_t
float var90 = uint16_t (0); // uint16_t -> float
double var91 = uint16_t (0); // uint16_t -> double


// u8vec2 ====> uvec2, u64vec2, vec2, dvec2
uvec2 var92 = u8vec2 (0); // u8vec2 -> uvec2
u64vec2 var93 = u8vec2 (0); // u8vec2 -> u64vec2
vec2 var94 = u8vec2 (0); // u8vec2 -> vec2
dvec2 var95 = u8vec2 (0); // u8vec2 -> dvec2


// u16vec2 ====> uvec2, u64vec2, vec2, dvec2
uvec2 var96 = u16vec2 (0); // u16vec2 -> uvec2
u64vec2 var97 = u16vec2 (0); // u16vec2 -> u64vec2
vec2 var98 = u16vec2 (0); // u16vec2 -> vec2
dvec2 var99 = u16vec2 (0); // u16vec2 -> dvec2


// u8vec3 ====> uvec3, u64vec3, vec3, dvec3
uvec3 var100 = u8vec3 (0); // u8vec3 -> uvec3
u64vec3 var101 = u8vec3 (0); // u8vec3 -> u64vec3
vec3 var102 = u8vec3 (0); // u8vec3 -> vec3
dvec3 var103 = u8vec3 (0); // u8vec3 -> dvec3


// u8vec4 ====> uvec4, u64vec4, vec4, dvec4
uvec4 var104 = u8vec4 (0); // u8vec4 -> uvec4
u64vec4 var105 = u8vec4 (0); // u8vec4 -> u64vec4
vec4 var106 = u8vec4 (0); // u8vec4 -> vec4
dvec4 var107 = u8vec4 (0); // u8vec4 -> dvec4


// uint64_t ====> double
double var108 = uint64_t (0); // uint64_t -> double


// u64vec2 ====> dvec2
dvec2 var109 = u64vec2 (0); // u64vec2 -> dvec2


// u64vec3 ====> dvec3
dvec3 var110 = u64vec3 (0); // u64vec3 -> dvec3


// u64vec4 ====> dvec4
dvec4 var111 = u64vec4 (0); // u64vec4 -> dvec4


// float ====> double
double var112 = float (0); // float -> double


// vec2 ====> dvec2
dvec2 var113 = vec2 (0); // vec2 -> dvec2


// vec3 ====> dvec3
dvec3 var114 = vec3 (0); // vec3 -> dvec3


// vec4 ====> dvec4
dvec4 var115 = vec4 (0); // vec4 -> dvec4


// float16_t ====> float, double
float var116 = float16_t (0); // float16_t -> float
double var117 = float16_t (0); // float16_t -> double


// f16vec2 ====> vec2, dvec2
vec2 var118 = f16vec2 (0); // f16vec2 -> vec2
dvec2 var119 = f16vec2 (0); // f16vec2 -> dvec2


// f16vec3 ====> vec3, dvec3
vec3 var120 = f16vec3 (0); // f16vec3 -> vec3
dvec3 var121 = f16vec3 (0); // f16vec3 -> dvec3


// f16vec4 ====> vec4, dvec4
vec4 var122 = f16vec4 (0); // f16vec4 -> vec4
dvec4 var123 = f16vec4 (0); // f16vec4 -> dvec4
}




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
flat in ivec2 Offsets[4];


// Test the builtins defined by ARB_gpu_shader5
void testbuiltinARB() {
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
var3 = textureGatherOffsets(svar24, var1, Offsets);
var11 = textureGatherOffsets(svar25, var1, Offsets);
var7 = textureGatherOffsets(svar26, var1, Offsets);
// Function Signature : gvec4 textureGatherOffsets(gsampler2DArray, vec2, ivec2[4])
var3 = textureGatherOffsets(svar33, var2, Offsets);
var11 = textureGatherOffsets(svar34, var2, Offsets);
var7 = textureGatherOffsets(svar35, var2, Offsets);
// Function Signature : gvec4 textureGatherOffsets(gsampler2DRect, vec2, ivec2[4])
var3 = textureGatherOffsets(svar27, var1, Offsets);
var11 = textureGatherOffsets(svar28, var1, Offsets);
var7 = textureGatherOffsets(svar29, var1, Offsets);
// Function Signature : vec4 textureGatherOffsets(sampler2DShadow, vec2, float, ivec2[4])
var3 = textureGatherOffsets(var16, var1, var0, Offsets);
// Function Signature : vec4 textureGatherOffsets(sampler2DArrayShadow, vec3, float, ivec2[4])
var3 = textureGatherOffsets(var18, var2, var0, Offsets);
// Function Signature : vec4 textureGatherOffsets(sampler2DRectShadow, vec2, float, ivec2[4])
var3 = textureGatherOffsets(var20, var1, var0, Offsets);
}

void testDataTypes()
{
    // 8 bit data types
    int8_t    var1;
    i8vec2    var2;
    i8vec3    var3;
    i8vec4    var4;
    uint8_t   var17;
    u8vec2    var18;
    u8vec3    var19;
    u8vec4    var20;
    // 16 bit data types
    int16_t   var5;
    i16vec2   var6;
    i16vec3   var7;
    i16vec4   var8;
    uint16_t  var21;
    u16vec2   var22;
    u16vec3   var23;
    u16vec4   var24;
    // 32 bit data types
    int32_t   var9;
    i32vec2   var10;
    i32vec3   var11;
    i32vec4   var12;
    uint32_t  var25; 
    u32vec2   var26;
    u32vec3   var27;
    u32vec4   var28;

    int64_t   var13;
    i64vec2   var14;
    i64vec3   var15;
    i64vec4   var16;
    uint64_t  var29;
    u64vec2   var30;
    u64vec3   var31;
    u64vec4   var32;

    float16_t var33;
    f16vec2   var34;
    f16vec3   var35;
    f16vec4   var36;
    float32_t var37;
    f32vec2   var38;
    f32vec3   var39;
    f32vec4   var40;

    float64_t var41; // needs GL_ARB_gpu_shader_fp64
    f64vec2   var42; // needs GL_ARB_gpu_shader_fp64
    f64vec3   var43; // needs GL_ARB_gpu_shader_fp64
    f64vec4   var44; // needs GL_ARB_gpu_shader_fp64
}


// Test explicit data in and out
flat in int8_t    var1_in;
flat in int16_t   var5_in;
flat in int32_t   var9_in;
flat in int64_t   var13_in;
flat in float16_t var33_in;
flat in float64_t var41_in;
flat in double var42_in;


out int8_t    var11_out;
out int16_t   var51_out;
out int32_t   var91_out;
//out int64_t   var131; // Error: TBD Needs to be confirmed
out float16_t var331_out;


void testConstructors()
{
      uint8_t uiv = uint8_t(1);
      double dv = 1.0;
      int16_t iv = int16_t(1);
      bool bv = false;
      float(uiv);      // converts an 8-bit uint value to a float
      int64_t(dv);     // converts a double value to a 64-bit int
      float64_t(iv);  // converts a 16-bit int value to a 64-bit float
      uint16_t(bv);      // converts a Boolean value to a 16-bit uint

}

void testBinaryOps()
{
int var0 = int( 0 );
uint var1 = uint( 1 );
int64_t var2 = int64_t( 2 );
uint64_t var3 = uint64_t( 3 );
float var4 = float( 4 );
double var5 = double( 5 );
ivec2 var6 = ivec2( 6 );
uvec2 var7 = uvec2( 7 );
i64vec2 var8 = i64vec2( 8 );
u64vec2 var9 = u64vec2( 9 );
vec2 var10 = vec2( 10 );
dvec2 var11 = dvec2( 11 );
ivec3 var12 = ivec3( 12 );
uvec3 var13 = uvec3( 13 );
i64vec3 var14 = i64vec3( 14 );
u64vec3 var15 = u64vec3( 15 );
vec3 var16 = vec3( 16 );
dvec3 var17 = dvec3( 17 );
ivec4 var18 = ivec4( 18 );
uvec4 var19 = uvec4( 19 );
i64vec4 var20 = i64vec4( 20 );
u64vec4 var21 = u64vec4( 21 );
vec4 var22 = vec4( 22 );
dvec4 var23 = dvec4( 23 );
int8_t var24 = int8_t( 24 );
int16_t var25 = int16_t( 25 );
i8vec2 var26 = i8vec2( 26 );
i16vec2 var27 = i16vec2( 27 );
i8vec3 var28 = i8vec3( 28 );
i16vec3 var29 = i16vec3( 29 );
i8vec4 var30 = i8vec4( 30 );
i16vec4 var31 = i16vec4( 31 );
uint8_t var32 = uint8_t( 32 );
uint16_t var33 = uint16_t( 33 );
u8vec2 var34 = u8vec2( 34 );
u16vec2 var35 = u16vec2( 35 );
u8vec3 var36 = u8vec3( 36 );
u8vec4 var37 = u8vec4( 37 );
float16_t var38 = float16_t( 38 );
f16vec2 var39 = f16vec2( 39 );
f16vec3 var40 = f16vec3( 40 );
f16vec4 var41 = f16vec4( 41 );
// Testing for binary operation +
// int ====> uint, int64_t, uint64_t, float, double
uint var42 = var1 + var0;
int64_t var43 = var2 + var0;
uint64_t var44 = var3 + var0;
float var45 = var4 + var0;
double var46 = var5 + var0;
// ivec2 ====> uvec2, i64vec2, u64vec2, vec2, dvec2
uvec2 var47 = var7 + var6;
i64vec2 var48 = var8 + var6;
u64vec2 var49 = var9 + var6;
vec2 var50 = var10 + var6;
dvec2 var51 = var11 + var6;
// ivec3 ====> uvec3, i64vec3, u64vec3, vec3, dvec3
uvec3 var52 = var13 + var12;
i64vec3 var53 = var14 + var12;
u64vec3 var54 = var15 + var12;
vec3 var55 = var16 + var12;
dvec3 var56 = var17 + var12;
// ivec4 ====> uvec4, i64vec4, u64vec4, vec4, dvec4
uvec4 var57 = var19 + var18;
i64vec4 var58 = var20 + var18;
u64vec4 var59 = var21 + var18;
vec4 var60 = var22 + var18;
dvec4 var61 = var23 + var18;
// int8_t ====> int, int64_t, uint, uint64_t, float, double
int var62 = var0 + var24;
int64_t var63 = var2 + var24;
uint var64 = var1 + var24;
uint64_t var65 = var3 + var24;
float var66 = var4 + var24;
double var67 = var5 + var24;
// int16_t ====> int, int64_t, uint, uint64_t, float, double
int var68 = var0 + var25;
int64_t var69 = var2 + var25;
uint var70 = var1 + var25;
uint64_t var71 = var3 + var25;
float var72 = var4 + var25;
double var73 = var5 + var25;
// i8vec2 ====> ivec2, i64vec2, uvec2, u64vec2, vec2, dvec2
ivec2 var74 = var6 + var26;
i64vec2 var75 = var8 + var26;
uvec2 var76 = var7 + var26;
u64vec2 var77 = var9 + var26;
vec2 var78 = var10 + var26;
dvec2 var79 = var11 + var26;
// i16vec2 ====> ivec2, i64vec2, uvec2, u64vec2, vec2, dvec2
ivec2 var80 = var6 + var27;
i64vec2 var81 = var8 + var27;
uvec2 var82 = var7 + var27;
u64vec2 var83 = var9 + var27;
vec2 var84 = var10 + var27;
dvec2 var85 = var11 + var27;
// i8vec3 ====> ivec3, i64vec3, uvec3, u64vec3, vec3, dvec3
ivec3 var86 = var12 + var28;
i64vec3 var87 = var14 + var28;
uvec3 var88 = var13 + var28;
u64vec3 var89 = var15 + var28;
vec3 var90 = var16 + var28;
dvec3 var91 = var17 + var28;
// i16vec3 ====> uvec3, u64vec3, vec3, dvec3
uvec3 var92 = var13 + var29;
u64vec3 var93 = var15 + var29;
vec3 var94 = var16 + var29;
dvec3 var95 = var17 + var29;
// i8vec4 ====> ivec4, i64vec4, uvec4, u64vec4, vec4, dvec4
ivec4 var96 = var18 + var30;
i64vec4 var97 = var20 + var30;
uvec4 var98 = var19 + var30;
u64vec4 var99 = var21 + var30;
vec4 var100 = var22 + var30;
dvec4 var101 = var23 + var30;
// i16vec4 ====> uvec4, u64vec4, vec4, dvec4
uvec4 var102 = var19 + var31;
u64vec4 var103 = var21 + var31;
vec4 var104 = var22 + var31;
dvec4 var105 = var23 + var31;
// int64_t ====> uint64_t, double
uint64_t var106 = var3 + var2;
double var107 = var5 + var2;
// i64vec2 ====> u64vec2, dvec2
u64vec2 var108 = var9 + var8;
dvec2 var109 = var11 + var8;
// i64vec3 ====> u64vec3, dvec3
u64vec3 var110 = var15 + var14;
dvec3 var111 = var17 + var14;
// i64vec4 ====> u64vec4, dvec4
u64vec4 var112 = var21 + var20;
dvec4 var113 = var23 + var20;
// uint ====> uint64_t, float, double
uint64_t var114 = var3 + var1;
float var115 = var4 + var1;
double var116 = var5 + var1;
// uvec2 ====> u64vec2, vec2, dvec2
u64vec2 var117 = var9 + var7;
vec2 var118 = var10 + var7;
dvec2 var119 = var11 + var7;
// uvec3 ====> u64vec3, vec3, dvec3
u64vec3 var120 = var15 + var13;
vec3 var121 = var16 + var13;
dvec3 var122 = var17 + var13;
// uvec4 ====> u64vec4, vec4, dvec4
u64vec4 var123 = var21 + var19;
vec4 var124 = var22 + var19;
dvec4 var125 = var23 + var19;
// uint8_t ====> uint, uint64_t, float, double
uint var126 = var1 + var32;
uint64_t var127 = var3 + var32;
float var128 = var4 + var32;
double var129 = var5 + var32;
// uint16_t ====> uint, uint64_t, float, double
uint var130 = var1 + var33;
uint64_t var131 = var3 + var33;
float var132 = var4 + var33;
double var133 = var5 + var33;
// u8vec2 ====> uvec2, u64vec2, vec2, dvec2
uvec2 var134 = var7 + var34;
u64vec2 var135 = var9 + var34;
vec2 var136 = var10 + var34;
dvec2 var137 = var11 + var34;
// u16vec2 ====> uvec2, u64vec2, vec2, dvec2
uvec2 var138 = var7 + var35;
u64vec2 var139 = var9 + var35;
vec2 var140 = var10 + var35;
dvec2 var141 = var11 + var35;
// u8vec3 ====> uvec3, u64vec3, vec3, dvec3
uvec3 var142 = var13 + var36;
u64vec3 var143 = var15 + var36;
vec3 var144 = var16 + var36;
dvec3 var145 = var17 + var36;
// u8vec4 ====> uvec4, u64vec4, vec4, dvec4
uvec4 var146 = var19 + var37;
u64vec4 var147 = var21 + var37;
vec4 var148 = var22 + var37;
dvec4 var149 = var23 + var37;
// uint64_t ====> double
double var150 = var5 + var3;
// u64vec2 ====> dvec2
dvec2 var151 = var11 + var9;
// u64vec3 ====> dvec3
dvec3 var152 = var17 + var15;
// u64vec4 ====> dvec4
dvec4 var153 = var23 + var21;
// float ====> double
double var154 = var5 + var4;
// vec2 ====> dvec2
dvec2 var155 = var11 + var10;
// vec3 ====> dvec3
dvec3 var156 = var17 + var16;
// vec4 ====> dvec4
dvec4 var157 = var23 + var22;
// float16_t ====> float, double
float var158 = var4 + var38;
double var159 = var5 + var38;
// f16vec2 ====> vec2, dvec2
vec2 var160 = var10 + var39;
dvec2 var161 = var11 + var39;
// f16vec3 ====> vec3, dvec3
vec3 var162 = var16 + var40;
dvec3 var163 = var17 + var40;
// f16vec4 ====> vec4, dvec4
vec4 var164 = var22 + var41;
dvec4 var165 = var23 + var41;
// Testing for binary operation -
// int ====> uint, int64_t, uint64_t, float, double
uint var166 = var1 - var0;
int64_t var167 = var2 - var0;
uint64_t var168 = var3 - var0;
float var169 = var4 - var0;
double var170 = var5 - var0;
// ivec2 ====> uvec2, i64vec2, u64vec2, vec2, dvec2
uvec2 var171 = var7 - var6;
i64vec2 var172 = var8 - var6;
u64vec2 var173 = var9 - var6;
vec2 var174 = var10 - var6;
dvec2 var175 = var11 - var6;
// ivec3 ====> uvec3, i64vec3, u64vec3, vec3, dvec3
uvec3 var176 = var13 - var12;
i64vec3 var177 = var14 - var12;
u64vec3 var178 = var15 - var12;
vec3 var179 = var16 - var12;
dvec3 var180 = var17 - var12;
// ivec4 ====> uvec4, i64vec4, u64vec4, vec4, dvec4
uvec4 var181 = var19 - var18;
i64vec4 var182 = var20 - var18;
u64vec4 var183 = var21 - var18;
vec4 var184 = var22 - var18;
dvec4 var185 = var23 - var18;
// int8_t ====> int, int64_t, uint, uint64_t, float, double
int var186 = var0 - var24;
int64_t var187 = var2 - var24;
uint var188 = var1 - var24;
uint64_t var189 = var3 - var24;
float var190 = var4 - var24;
double var191 = var5 - var24;
// int16_t ====> int, int64_t, uint, uint64_t, float, double
int var192 = var0 - var25;
int64_t var193 = var2 - var25;
uint var194 = var1 - var25;
uint64_t var195 = var3 - var25;
float var196 = var4 - var25;
double var197 = var5 - var25;
// i8vec2 ====> ivec2, i64vec2, uvec2, u64vec2, vec2, dvec2
ivec2 var198 = var6 - var26;
i64vec2 var199 = var8 - var26;
uvec2 var200 = var7 - var26;
u64vec2 var201 = var9 - var26;
vec2 var202 = var10 - var26;
dvec2 var203 = var11 - var26;
// i16vec2 ====> ivec2, i64vec2, uvec2, u64vec2, vec2, dvec2
ivec2 var204 = var6 - var27;
i64vec2 var205 = var8 - var27;
uvec2 var206 = var7 - var27;
u64vec2 var207 = var9 - var27;
vec2 var208 = var10 - var27;
dvec2 var209 = var11 - var27;
// i8vec3 ====> ivec3, i64vec3, uvec3, u64vec3, vec3, dvec3
ivec3 var210 = var12 - var28;
i64vec3 var211 = var14 - var28;
uvec3 var212 = var13 - var28;
u64vec3 var213 = var15 - var28;
vec3 var214 = var16 - var28;
dvec3 var215 = var17 - var28;
// i16vec3 ====> uvec3, u64vec3, vec3, dvec3
uvec3 var216 = var13 - var29;
u64vec3 var217 = var15 - var29;
vec3 var218 = var16 - var29;
dvec3 var219 = var17 - var29;
// i8vec4 ====> ivec4, i64vec4, uvec4, u64vec4, vec4, dvec4
ivec4 var220 = var18 - var30;
i64vec4 var221 = var20 - var30;
uvec4 var222 = var19 - var30;
u64vec4 var223 = var21 - var30;
vec4 var224 = var22 - var30;
dvec4 var225 = var23 - var30;
// i16vec4 ====> uvec4, u64vec4, vec4, dvec4
uvec4 var226 = var19 - var31;
u64vec4 var227 = var21 - var31;
vec4 var228 = var22 - var31;
dvec4 var229 = var23 - var31;
// int64_t ====> uint64_t, double
uint64_t var230 = var3 - var2;
double var231 = var5 - var2;
// i64vec2 ====> u64vec2, dvec2
u64vec2 var232 = var9 - var8;
dvec2 var233 = var11 - var8;
// i64vec3 ====> u64vec3, dvec3
u64vec3 var234 = var15 - var14;
dvec3 var235 = var17 - var14;
// i64vec4 ====> u64vec4, dvec4
u64vec4 var236 = var21 - var20;
dvec4 var237 = var23 - var20;
// uint ====> uint64_t, float, double
uint64_t var238 = var3 - var1;
float var239 = var4 - var1;
double var240 = var5 - var1;
// uvec2 ====> u64vec2, vec2, dvec2
u64vec2 var241 = var9 - var7;
vec2 var242 = var10 - var7;
dvec2 var243 = var11 - var7;
// uvec3 ====> u64vec3, vec3, dvec3
u64vec3 var244 = var15 - var13;
vec3 var245 = var16 - var13;
dvec3 var246 = var17 - var13;
// uvec4 ====> u64vec4, vec4, dvec4
u64vec4 var247 = var21 - var19;
vec4 var248 = var22 - var19;
dvec4 var249 = var23 - var19;
// uint8_t ====> uint, uint64_t, float, double
uint var250 = var1 - var32;
uint64_t var251 = var3 - var32;
float var252 = var4 - var32;
double var253 = var5 - var32;
// uint16_t ====> uint, uint64_t, float, double
uint var254 = var1 - var33;
uint64_t var255 = var3 - var33;
float var256 = var4 - var33;
double var257 = var5 - var33;
// u8vec2 ====> uvec2, u64vec2, vec2, dvec2
uvec2 var258 = var7 - var34;
u64vec2 var259 = var9 - var34;
vec2 var260 = var10 - var34;
dvec2 var261 = var11 - var34;
// u16vec2 ====> uvec2, u64vec2, vec2, dvec2
uvec2 var262 = var7 - var35;
u64vec2 var263 = var9 - var35;
vec2 var264 = var10 - var35;
dvec2 var265 = var11 - var35;
// u8vec3 ====> uvec3, u64vec3, vec3, dvec3
uvec3 var266 = var13 - var36;
u64vec3 var267 = var15 - var36;
vec3 var268 = var16 - var36;
dvec3 var269 = var17 - var36;
// u8vec4 ====> uvec4, u64vec4, vec4, dvec4
uvec4 var270 = var19 - var37;
u64vec4 var271 = var21 - var37;
vec4 var272 = var22 - var37;
dvec4 var273 = var23 - var37;
// uint64_t ====> double
double var274 = var5 - var3;
// u64vec2 ====> dvec2
dvec2 var275 = var11 - var9;
// u64vec3 ====> dvec3
dvec3 var276 = var17 - var15;
// u64vec4 ====> dvec4
dvec4 var277 = var23 - var21;
// float ====> double
double var278 = var5 - var4;
// vec2 ====> dvec2
dvec2 var279 = var11 - var10;
// vec3 ====> dvec3
dvec3 var280 = var17 - var16;
// vec4 ====> dvec4
dvec4 var281 = var23 - var22;
// float16_t ====> float, double
float var282 = var4 - var38;
double var283 = var5 - var38;
// f16vec2 ====> vec2, dvec2
vec2 var284 = var10 - var39;
dvec2 var285 = var11 - var39;
// f16vec3 ====> vec3, dvec3
vec3 var286 = var16 - var40;
dvec3 var287 = var17 - var40;
// f16vec4 ====> vec4, dvec4
vec4 var288 = var22 - var41;
dvec4 var289 = var23 - var41;
// Testing for binary operation *
// int ====> uint, int64_t, uint64_t, float, double
uint var290 = var1 * var0;
int64_t var291 = var2 * var0;
uint64_t var292 = var3 * var0;
float var293 = var4 * var0;
double var294 = var5 * var0;
// ivec2 ====> uvec2, i64vec2, u64vec2, vec2, dvec2
uvec2 var295 = var7 * var6;
i64vec2 var296 = var8 * var6;
u64vec2 var297 = var9 * var6;
vec2 var298 = var10 * var6;
dvec2 var299 = var11 * var6;
// ivec3 ====> uvec3, i64vec3, u64vec3, vec3, dvec3
uvec3 var300 = var13 * var12;
i64vec3 var301 = var14 * var12;
u64vec3 var302 = var15 * var12;
vec3 var303 = var16 * var12;
dvec3 var304 = var17 * var12;
// ivec4 ====> uvec4, i64vec4, u64vec4, vec4, dvec4
uvec4 var305 = var19 * var18;
i64vec4 var306 = var20 * var18;
u64vec4 var307 = var21 * var18;
vec4 var308 = var22 * var18;
dvec4 var309 = var23 * var18;
// int8_t ====> int, int64_t, uint, uint64_t, float, double
int var310 = var0 * var24;
int64_t var311 = var2 * var24;
uint var312 = var1 * var24;
uint64_t var313 = var3 * var24;
float var314 = var4 * var24;
double var315 = var5 * var24;
// int16_t ====> int, int64_t, uint, uint64_t, float, double
int var316 = var0 * var25;
int64_t var317 = var2 * var25;
uint var318 = var1 * var25;
uint64_t var319 = var3 * var25;
float var320 = var4 * var25;
double var321 = var5 * var25;
// i8vec2 ====> ivec2, i64vec2, uvec2, u64vec2, vec2, dvec2
ivec2 var322 = var6 * var26;
i64vec2 var323 = var8 * var26;
uvec2 var324 = var7 * var26;
u64vec2 var325 = var9 * var26;
vec2 var326 = var10 * var26;
dvec2 var327 = var11 * var26;
// i16vec2 ====> ivec2, i64vec2, uvec2, u64vec2, vec2, dvec2
ivec2 var328 = var6 * var27;
i64vec2 var329 = var8 * var27;
uvec2 var330 = var7 * var27;
u64vec2 var331 = var9 * var27;
vec2 var332 = var10 * var27;
dvec2 var333 = var11 * var27;
// i8vec3 ====> ivec3, i64vec3, uvec3, u64vec3, vec3, dvec3
ivec3 var334 = var12 * var28;
i64vec3 var335 = var14 * var28;
uvec3 var336 = var13 * var28;
u64vec3 var337 = var15 * var28;
vec3 var338 = var16 * var28;
dvec3 var339 = var17 * var28;
// i16vec3 ====> uvec3, u64vec3, vec3, dvec3
uvec3 var340 = var13 * var29;
u64vec3 var341 = var15 * var29;
vec3 var342 = var16 * var29;
dvec3 var343 = var17 * var29;
// i8vec4 ====> ivec4, i64vec4, uvec4, u64vec4, vec4, dvec4
ivec4 var344 = var18 * var30;
i64vec4 var345 = var20 * var30;
uvec4 var346 = var19 * var30;
u64vec4 var347 = var21 * var30;
vec4 var348 = var22 * var30;
dvec4 var349 = var23 * var30;
// i16vec4 ====> uvec4, u64vec4, vec4, dvec4
uvec4 var350 = var19 * var31;
u64vec4 var351 = var21 * var31;
vec4 var352 = var22 * var31;
dvec4 var353 = var23 * var31;
// int64_t ====> uint64_t, double
uint64_t var354 = var3 * var2;
double var355 = var5 * var2;
// i64vec2 ====> u64vec2, dvec2
u64vec2 var356 = var9 * var8;
dvec2 var357 = var11 * var8;
// i64vec3 ====> u64vec3, dvec3
u64vec3 var358 = var15 * var14;
dvec3 var359 = var17 * var14;
// i64vec4 ====> u64vec4, dvec4
u64vec4 var360 = var21 * var20;
dvec4 var361 = var23 * var20;
// uint ====> uint64_t, float, double
uint64_t var362 = var3 * var1;
float var363 = var4 * var1;
double var364 = var5 * var1;
// uvec2 ====> u64vec2, vec2, dvec2
u64vec2 var365 = var9 * var7;
vec2 var366 = var10 * var7;
dvec2 var367 = var11 * var7;
// uvec3 ====> u64vec3, vec3, dvec3
u64vec3 var368 = var15 * var13;
vec3 var369 = var16 * var13;
dvec3 var370 = var17 * var13;
// uvec4 ====> u64vec4, vec4, dvec4
u64vec4 var371 = var21 * var19;
vec4 var372 = var22 * var19;
dvec4 var373 = var23 * var19;
// uint8_t ====> uint, uint64_t, float, double
uint var374 = var1 * var32;
uint64_t var375 = var3 * var32;
float var376 = var4 * var32;
double var377 = var5 * var32;
// uint16_t ====> uint, uint64_t, float, double
uint var378 = var1 * var33;
uint64_t var379 = var3 * var33;
float var380 = var4 * var33;
double var381 = var5 * var33;
// u8vec2 ====> uvec2, u64vec2, vec2, dvec2
uvec2 var382 = var7 * var34;
u64vec2 var383 = var9 * var34;
vec2 var384 = var10 * var34;
dvec2 var385 = var11 * var34;
// u16vec2 ====> uvec2, u64vec2, vec2, dvec2
uvec2 var386 = var7 * var35;
u64vec2 var387 = var9 * var35;
vec2 var388 = var10 * var35;
dvec2 var389 = var11 * var35;
// u8vec3 ====> uvec3, u64vec3, vec3, dvec3
uvec3 var390 = var13 * var36;
u64vec3 var391 = var15 * var36;
vec3 var392 = var16 * var36;
dvec3 var393 = var17 * var36;
// u8vec4 ====> uvec4, u64vec4, vec4, dvec4
uvec4 var394 = var19 * var37;
u64vec4 var395 = var21 * var37;
vec4 var396 = var22 * var37;
dvec4 var397 = var23 * var37;
// uint64_t ====> double
double var398 = var5 * var3;
// u64vec2 ====> dvec2
dvec2 var399 = var11 * var9;
// u64vec3 ====> dvec3
dvec3 var400 = var17 * var15;
// u64vec4 ====> dvec4
dvec4 var401 = var23 * var21;
// float ====> double
double var402 = var5 * var4;
// vec2 ====> dvec2
dvec2 var403 = var11 * var10;
// vec3 ====> dvec3
dvec3 var404 = var17 * var16;
// vec4 ====> dvec4
dvec4 var405 = var23 * var22;
// float16_t ====> float, double
float var406 = var4 * var38;
double var407 = var5 * var38;
// f16vec2 ====> vec2, dvec2
vec2 var408 = var10 * var39;
dvec2 var409 = var11 * var39;
// f16vec3 ====> vec3, dvec3
vec3 var410 = var16 * var40;
dvec3 var411 = var17 * var40;
// f16vec4 ====> vec4, dvec4
vec4 var412 = var22 * var41;
dvec4 var413 = var23 * var41;
}

void testModuloOps() {
int var0 = int( 0 );
uint var1 = uint( 1 );
int64_t var2 = int64_t( 2 );
uint64_t var3 = uint64_t( 3 );
float var4 = float( 4 );
double var5 = double( 5 );
ivec2 var6 = ivec2( 6 );
uvec2 var7 = uvec2( 7 );
i64vec2 var8 = i64vec2( 8 );
u64vec2 var9 = u64vec2( 9 );
vec2 var10 = vec2( 10 );
dvec2 var11 = dvec2( 11 );
ivec3 var12 = ivec3( 12 );
uvec3 var13 = uvec3( 13 );
i64vec3 var14 = i64vec3( 14 );
u64vec3 var15 = u64vec3( 15 );
vec3 var16 = vec3( 16 );
dvec3 var17 = dvec3( 17 );
ivec4 var18 = ivec4( 18 );
uvec4 var19 = uvec4( 19 );
i64vec4 var20 = i64vec4( 20 );
u64vec4 var21 = u64vec4( 21 );
vec4 var22 = vec4( 22 );
dvec4 var23 = dvec4( 23 );
int8_t var24 = int8_t( 24 );
int16_t var25 = int16_t( 25 );
i8vec2 var26 = i8vec2( 26 );
i16vec2 var27 = i16vec2( 27 );
i8vec3 var28 = i8vec3( 28 );
i16vec3 var29 = i16vec3( 29 );
i8vec4 var30 = i8vec4( 30 );
i16vec4 var31 = i16vec4( 31 );
uint8_t var32 = uint8_t( 32 );
uint16_t var33 = uint16_t( 33 );
u8vec2 var34 = u8vec2( 34 );
u16vec2 var35 = u16vec2( 35 );
u8vec3 var36 = u8vec3( 36 );
u8vec4 var37 = u8vec4( 37 );
float16_t var38 = float16_t( 38 );
f16vec2 var39 = f16vec2( 39 );
f16vec3 var40 = f16vec3( 40 );
f16vec4 var41 = f16vec4( 41 );
uint var42 = var1 % var0; // uint%int
int64_t var43 = var2 % var0; // int64_t%int
uint64_t var44 = var3 % var0; // uint64_t%int
uvec2 var45 = var7 % var6; // uvec2%ivec2
i64vec2 var46 = var8 % var6; // i64vec2%ivec2
u64vec2 var47 = var9 % var6; // u64vec2%ivec2
uvec3 var48 = var13 % var12; // uvec3%ivec3
i64vec3 var49 = var14 % var12; // i64vec3%ivec3
u64vec3 var50 = var15 % var12; // u64vec3%ivec3
uvec4 var51 = var19 % var18; // uvec4%ivec4
i64vec4 var52 = var20 % var18; // i64vec4%ivec4
u64vec4 var53 = var21 % var18; // u64vec4%ivec4
int var54 = var0 % var24; // int%int8_t
int64_t var55 = var2 % var24; // int64_t%int8_t
uint var56 = var1 % var24; // uint%int8_t
uint64_t var57 = var3 % var24; // uint64_t%int8_t
int var58 = var0 % var25; // int%int16_t
int64_t var59 = var2 % var25; // int64_t%int16_t
uint var60 = var1 % var25; // uint%int16_t
uint64_t var61 = var3 % var25; // uint64_t%int16_t
ivec2 var62 = var6 % var26; // ivec2%i8vec2
i64vec2 var63 = var8 % var26; // i64vec2%i8vec2
uvec2 var64 = var7 % var26; // uvec2%i8vec2
u64vec2 var65 = var9 % var26; // u64vec2%i8vec2
ivec2 var66 = var6 % var27; // ivec2%i16vec2
i64vec2 var67 = var8 % var27; // i64vec2%i16vec2
uvec2 var68 = var7 % var27; // uvec2%i16vec2
u64vec2 var69 = var9 % var27; // u64vec2%i16vec2
ivec3 var70 = var12 % var28; // ivec3%i8vec3
i64vec3 var71 = var14 % var28; // i64vec3%i8vec3
uvec3 var72 = var13 % var28; // uvec3%i8vec3
u64vec3 var73 = var15 % var28; // u64vec3%i8vec3
uvec3 var74 = var13 % var29; // uvec3%i16vec3
u64vec3 var75 = var15 % var29; // u64vec3%i16vec3
ivec4 var76 = var18 % var30; // ivec4%i8vec4
i64vec4 var77 = var20 % var30; // i64vec4%i8vec4
uvec4 var78 = var19 % var30; // uvec4%i8vec4
u64vec4 var79 = var21 % var30; // u64vec4%i8vec4
uvec4 var80 = var19 % var31; // uvec4%i16vec4
u64vec4 var81 = var21 % var31; // u64vec4%i16vec4
uint64_t var82 = var3 % var2; // uint64_t%int64_t
u64vec2 var83 = var9 % var8; // u64vec2%i64vec2
u64vec3 var84 = var15 % var14; // u64vec3%i64vec3
u64vec4 var85 = var21 % var20; // u64vec4%i64vec4
uint64_t var86 = var3 % var1; // uint64_t%uint
u64vec2 var87 = var9 % var7; // u64vec2%uvec2
u64vec3 var88 = var15 % var13; // u64vec3%uvec3
u64vec4 var89 = var21 % var19; // u64vec4%uvec4
uint var90 = var1 % var32; // uint%uint8_t
uint64_t var91 = var3 % var32; // uint64_t%uint8_t
uint var92 = var1 % var33; // uint%uint16_t
uint64_t var93 = var3 % var33; // uint64_t%uint16_t
uvec2 var94 = var7 % var34; // uvec2%u8vec2
u64vec2 var95 = var9 % var34; // u64vec2%u8vec2
uvec2 var96 = var7 % var35; // uvec2%u16vec2
u64vec2 var97 = var9 % var35; // u64vec2%u16vec2
uvec3 var98 = var13 % var36; // uvec3%u8vec3
u64vec3 var99 = var15 % var36; // u64vec3%u8vec3
uvec4 var100 = var19 % var37; // uvec4%u8vec4
u64vec4 var101 = var21 % var37; // u64vec4%u8vec4
}


void testUnaryOps() {
int var0 = int( 0 );
uint var1 = uint( 1 );
int64_t var2 = int64_t( 2 );
uint64_t var3 = uint64_t( 3 );
float var4 = float( 4 );
double var5 = double( 5 );
ivec2 var6 = ivec2( 6 );
uvec2 var7 = uvec2( 7 );
i64vec2 var8 = i64vec2( 8 );
u64vec2 var9 = u64vec2( 9 );
vec2 var10 = vec2( 10 );
dvec2 var11 = dvec2( 11 );
ivec3 var12 = ivec3( 12 );
uvec3 var13 = uvec3( 13 );
i64vec3 var14 = i64vec3( 14 );
u64vec3 var15 = u64vec3( 15 );
vec3 var16 = vec3( 16 );
dvec3 var17 = dvec3( 17 );
ivec4 var18 = ivec4( 18 );
uvec4 var19 = uvec4( 19 );
i64vec4 var20 = i64vec4( 20 );
u64vec4 var21 = u64vec4( 21 );
vec4 var22 = vec4( 22 );
dvec4 var23 = dvec4( 23 );
int8_t var24 = int8_t( 24 );
int16_t var25 = int16_t( 25 );
i8vec2 var26 = i8vec2( 26 );
i16vec2 var27 = i16vec2( 27 );
i8vec3 var28 = i8vec3( 28 );
i16vec3 var29 = i16vec3( 29 );
i8vec4 var30 = i8vec4( 30 );
i16vec4 var31 = i16vec4( 31 );
uint8_t var32 = uint8_t( 32 );
uint16_t var33 = uint16_t( 33 );
u8vec2 var34 = u8vec2( 34 );
u16vec2 var35 = u16vec2( 35 );
u8vec3 var36 = u8vec3( 36 );
u8vec4 var37 = u8vec4( 37 );
float16_t var38 = float16_t( 38 );
f16vec2 var39 = f16vec2( 39 );
f16vec3 var40 = f16vec3( 40 );
f16vec4 var41 = f16vec4( 41 );
// int ====> uint, int64_t, uint64_t, float, double
uint var42 = -var1; //uint
int64_t var43 = -var2; //int64_t
uint64_t var44 = -var3; //uint64_t
float var45 = -var4; //float
double var46 = -var5; //double
// ivec2 ====> uvec2, i64vec2, u64vec2, vec2, dvec2
uvec2 var47 = -var7; //uvec2
i64vec2 var48 = -var8; //i64vec2
u64vec2 var49 = -var9; //u64vec2
vec2 var50 = -var10; //vec2
dvec2 var51 = -var11; //dvec2
// ivec3 ====> uvec3, i64vec3, u64vec3, vec3, dvec3
uvec3 var52 = -var13; //uvec3
i64vec3 var53 = -var14; //i64vec3
u64vec3 var54 = -var15; //u64vec3
vec3 var55 = -var16; //vec3
dvec3 var56 = -var17; //dvec3
// ivec4 ====> uvec4, i64vec4, u64vec4, vec4, dvec4
uvec4 var57 = -var19; //uvec4
i64vec4 var58 = -var20; //i64vec4
u64vec4 var59 = -var21; //u64vec4
vec4 var60 = -var22; //vec4
dvec4 var61 = -var23; //dvec4
// int8_t ====> int, int64_t, uint, uint64_t, float, double
int var62 = -var0; //int
int64_t var63 = -var2; //int64_t
uint var64 = -var1; //uint
uint64_t var65 = -var3; //uint64_t
float var66 = -var4; //float
double var67 = -var5; //double
// int16_t ====> int, int64_t, uint, uint64_t, float, double
int var68 = -var0; //int
int64_t var69 = -var2; //int64_t
uint var70 = -var1; //uint
uint64_t var71 = -var3; //uint64_t
float var72 = -var4; //float
double var73 = -var5; //double
// i8vec2 ====> ivec2, i64vec2, uvec2, u64vec2, vec2, dvec2
ivec2 var74 = -var6; //ivec2
i64vec2 var75 = -var8; //i64vec2
uvec2 var76 = -var7; //uvec2
u64vec2 var77 = -var9; //u64vec2
vec2 var78 = -var10; //vec2
dvec2 var79 = -var11; //dvec2
// i16vec2 ====> ivec2, i64vec2, uvec2, u64vec2, vec2, dvec2
ivec2 var80 = -var6; //ivec2
i64vec2 var81 = -var8; //i64vec2
uvec2 var82 = -var7; //uvec2
u64vec2 var83 = -var9; //u64vec2
vec2 var84 = -var10; //vec2
dvec2 var85 = -var11; //dvec2
// i8vec3 ====> ivec3, i64vec3, uvec3, u64vec3, vec3, dvec3
ivec3 var86 = -var12; //ivec3
i64vec3 var87 = -var14; //i64vec3
uvec3 var88 = -var13; //uvec3
u64vec3 var89 = -var15; //u64vec3
vec3 var90 = -var16; //vec3
dvec3 var91 = -var17; //dvec3
// i16vec3 ====> uvec3, u64vec3, vec3, dvec3
uvec3 var92 = -var13; //uvec3
u64vec3 var93 = -var15; //u64vec3
vec3 var94 = -var16; //vec3
dvec3 var95 = -var17; //dvec3
// i8vec4 ====> ivec4, i64vec4, uvec4, u64vec4, vec4, dvec4
ivec4 var96 = -var18; //ivec4
i64vec4 var97 = -var20; //i64vec4
uvec4 var98 = -var19; //uvec4
u64vec4 var99 = -var21; //u64vec4
vec4 var100 = -var22; //vec4
dvec4 var101 = -var23; //dvec4
// i16vec4 ====> uvec4, u64vec4, vec4, dvec4
uvec4 var102 = -var19; //uvec4
u64vec4 var103 = -var21; //u64vec4
vec4 var104 = -var22; //vec4
dvec4 var105 = -var23; //dvec4
// int64_t ====> uint64_t, double
uint64_t var106 = -var3; //uint64_t
double var107 = -var5; //double
// i64vec2 ====> u64vec2, dvec2
u64vec2 var108 = -var9; //u64vec2
dvec2 var109 = -var11; //dvec2
// i64vec3 ====> u64vec3, dvec3
u64vec3 var110 = -var15; //u64vec3
dvec3 var111 = -var17; //dvec3
// i64vec4 ====> u64vec4, dvec4
u64vec4 var112 = -var21; //u64vec4
dvec4 var113 = -var23; //dvec4
// uint ====> uint64_t, float, double
uint64_t var114 = -var3; //uint64_t
float var115 = -var4; //float
double var116 = -var5; //double
// uvec2 ====> u64vec2, vec2, dvec2
u64vec2 var117 = -var9; //u64vec2
vec2 var118 = -var10; //vec2
dvec2 var119 = -var11; //dvec2
// uvec3 ====> u64vec3, vec3, dvec3
u64vec3 var120 = -var15; //u64vec3
vec3 var121 = -var16; //vec3
dvec3 var122 = -var17; //dvec3
// uvec4 ====> u64vec4, vec4, dvec4
u64vec4 var123 = -var21; //u64vec4
vec4 var124 = -var22; //vec4
dvec4 var125 = -var23; //dvec4
// uint8_t ====> uint, uint64_t, float, double
uint var126 = -var1; //uint
uint64_t var127 = -var3; //uint64_t
float var128 = -var4; //float
double var129 = -var5; //double
// uint16_t ====> uint, uint64_t, float, double
uint var130 = -var1; //uint
uint64_t var131 = -var3; //uint64_t
float var132 = -var4; //float
double var133 = -var5; //double
// u8vec2 ====> uvec2, u64vec2, vec2, dvec2
uvec2 var134 = -var7; //uvec2
u64vec2 var135 = -var9; //u64vec2
vec2 var136 = -var10; //vec2
dvec2 var137 = -var11; //dvec2
// u16vec2 ====> uvec2, u64vec2, vec2, dvec2
uvec2 var138 = -var7; //uvec2
u64vec2 var139 = -var9; //u64vec2
vec2 var140 = -var10; //vec2
dvec2 var141 = -var11; //dvec2
// u8vec3 ====> uvec3, u64vec3, vec3, dvec3
uvec3 var142 = -var13; //uvec3
u64vec3 var143 = -var15; //u64vec3
vec3 var144 = -var16; //vec3
dvec3 var145 = -var17; //dvec3
// u8vec4 ====> uvec4, u64vec4, vec4, dvec4
uvec4 var146 = -var19; //uvec4
u64vec4 var147 = -var21; //u64vec4
vec4 var148 = -var22; //vec4
dvec4 var149 = -var23; //dvec4
// uint64_t ====> double
double var150 = -var5; //double
// u64vec2 ====> dvec2
dvec2 var151 = -var11; //dvec2
// u64vec3 ====> dvec3
dvec3 var152 = -var17; //dvec3
// u64vec4 ====> dvec4
dvec4 var153 = -var23; //dvec4
// float ====> double
double var154 = -var5; //double
// vec2 ====> dvec2
dvec2 var155 = -var11; //dvec2
// vec3 ====> dvec3
dvec3 var156 = -var17; //dvec3
// vec4 ====> dvec4
dvec4 var157 = -var23; //dvec4
// float16_t ====> float, double
float var158 = -var4; //float
double var159 = -var5; //double
// f16vec2 ====> vec2, dvec2
vec2 var160 = -var10; //vec2
dvec2 var161 = -var11; //dvec2
// f16vec3 ====> vec3, dvec3
vec3 var162 = -var16; //vec3
dvec3 var163 = -var17; //dvec3
// f16vec4 ====> vec4, dvec4
vec4 var164 = -var22; //vec4
dvec4 var165 = -var23; //dvec4
// int ====> uint, int64_t, uint64_t, float, double
uint var166 = ++var1; //uint
int64_t var167 = ++var2; //int64_t
uint64_t var168 = ++var3; //uint64_t
float var169 = ++var4; //float
double var170 = ++var5; //double
// ivec2 ====> uvec2, i64vec2, u64vec2, vec2, dvec2
uvec2 var171 = ++var7; //uvec2
i64vec2 var172 = ++var8; //i64vec2
u64vec2 var173 = ++var9; //u64vec2
vec2 var174 = ++var10; //vec2
dvec2 var175 = ++var11; //dvec2
// ivec3 ====> uvec3, i64vec3, u64vec3, vec3, dvec3
uvec3 var176 = ++var13; //uvec3
i64vec3 var177 = ++var14; //i64vec3
u64vec3 var178 = ++var15; //u64vec3
vec3 var179 = ++var16; //vec3
dvec3 var180 = ++var17; //dvec3
// ivec4 ====> uvec4, i64vec4, u64vec4, vec4, dvec4
uvec4 var181 = ++var19; //uvec4
i64vec4 var182 = ++var20; //i64vec4
u64vec4 var183 = ++var21; //u64vec4
vec4 var184 = ++var22; //vec4
dvec4 var185 = ++var23; //dvec4
// int8_t ====> int, int64_t, uint, uint64_t, float, double
int var186 = ++var0; //int
int64_t var187 = ++var2; //int64_t
uint var188 = ++var1; //uint
uint64_t var189 = ++var3; //uint64_t
float var190 = ++var4; //float
double var191 = ++var5; //double
// int16_t ====> int, int64_t, uint, uint64_t, float, double
int var192 = ++var0; //int
int64_t var193 = ++var2; //int64_t
uint var194 = ++var1; //uint
uint64_t var195 = ++var3; //uint64_t
float var196 = ++var4; //float
double var197 = ++var5; //double
// i8vec2 ====> ivec2, i64vec2, uvec2, u64vec2, vec2, dvec2
ivec2 var198 = ++var6; //ivec2
i64vec2 var199 = ++var8; //i64vec2
uvec2 var200 = ++var7; //uvec2
u64vec2 var201 = ++var9; //u64vec2
vec2 var202 = ++var10; //vec2
dvec2 var203 = ++var11; //dvec2
// i16vec2 ====> ivec2, i64vec2, uvec2, u64vec2, vec2, dvec2
ivec2 var204 = ++var6; //ivec2
i64vec2 var205 = ++var8; //i64vec2
uvec2 var206 = ++var7; //uvec2
u64vec2 var207 = ++var9; //u64vec2
vec2 var208 = ++var10; //vec2
dvec2 var209 = ++var11; //dvec2
// i8vec3 ====> ivec3, i64vec3, uvec3, u64vec3, vec3, dvec3
ivec3 var210 = ++var12; //ivec3
i64vec3 var211 = ++var14; //i64vec3
uvec3 var212 = ++var13; //uvec3
u64vec3 var213 = ++var15; //u64vec3
vec3 var214 = ++var16; //vec3
dvec3 var215 = ++var17; //dvec3
// i16vec3 ====> uvec3, u64vec3, vec3, dvec3
uvec3 var216 = ++var13; //uvec3
u64vec3 var217 = ++var15; //u64vec3
vec3 var218 = ++var16; //vec3
dvec3 var219 = ++var17; //dvec3
// i8vec4 ====> ivec4, i64vec4, uvec4, u64vec4, vec4, dvec4
ivec4 var220 = ++var18; //ivec4
i64vec4 var221 = ++var20; //i64vec4
uvec4 var222 = ++var19; //uvec4
u64vec4 var223 = ++var21; //u64vec4
vec4 var224 = ++var22; //vec4
dvec4 var225 = ++var23; //dvec4
// i16vec4 ====> uvec4, u64vec4, vec4, dvec4
uvec4 var226 = ++var19; //uvec4
u64vec4 var227 = ++var21; //u64vec4
vec4 var228 = ++var22; //vec4
dvec4 var229 = ++var23; //dvec4
// int64_t ====> uint64_t, double
uint64_t var230 = ++var3; //uint64_t
double var231 = ++var5; //double
// i64vec2 ====> u64vec2, dvec2
u64vec2 var232 = ++var9; //u64vec2
dvec2 var233 = ++var11; //dvec2
// i64vec3 ====> u64vec3, dvec3
u64vec3 var234 = ++var15; //u64vec3
dvec3 var235 = ++var17; //dvec3
// i64vec4 ====> u64vec4, dvec4
u64vec4 var236 = ++var21; //u64vec4
dvec4 var237 = ++var23; //dvec4
// uint ====> uint64_t, float, double
uint64_t var238 = ++var3; //uint64_t
float var239 = ++var4; //float
double var240 = ++var5; //double
// uvec2 ====> u64vec2, vec2, dvec2
u64vec2 var241 = ++var9; //u64vec2
vec2 var242 = ++var10; //vec2
dvec2 var243 = ++var11; //dvec2
// uvec3 ====> u64vec3, vec3, dvec3
u64vec3 var244 = ++var15; //u64vec3
vec3 var245 = ++var16; //vec3
dvec3 var246 = ++var17; //dvec3
// uvec4 ====> u64vec4, vec4, dvec4
u64vec4 var247 = ++var21; //u64vec4
vec4 var248 = ++var22; //vec4
dvec4 var249 = ++var23; //dvec4
// uint8_t ====> uint, uint64_t, float, double
uint var250 = ++var1; //uint
uint64_t var251 = ++var3; //uint64_t
float var252 = ++var4; //float
double var253 = ++var5; //double
// uint16_t ====> uint, uint64_t, float, double
uint var254 = ++var1; //uint
uint64_t var255 = ++var3; //uint64_t
float var256 = ++var4; //float
double var257 = ++var5; //double
// u8vec2 ====> uvec2, u64vec2, vec2, dvec2
uvec2 var258 = ++var7; //uvec2
u64vec2 var259 = ++var9; //u64vec2
vec2 var260 = ++var10; //vec2
dvec2 var261 = ++var11; //dvec2
// u16vec2 ====> uvec2, u64vec2, vec2, dvec2
uvec2 var262 = ++var7; //uvec2
u64vec2 var263 = ++var9; //u64vec2
vec2 var264 = ++var10; //vec2
dvec2 var265 = ++var11; //dvec2
// u8vec3 ====> uvec3, u64vec3, vec3, dvec3
uvec3 var266 = ++var13; //uvec3
u64vec3 var267 = ++var15; //u64vec3
vec3 var268 = ++var16; //vec3
dvec3 var269 = ++var17; //dvec3
// u8vec4 ====> uvec4, u64vec4, vec4, dvec4
uvec4 var270 = ++var19; //uvec4
u64vec4 var271 = ++var21; //u64vec4
vec4 var272 = ++var22; //vec4
dvec4 var273 = ++var23; //dvec4
// uint64_t ====> double
double var274 = ++var5; //double
// u64vec2 ====> dvec2
dvec2 var275 = ++var11; //dvec2
// u64vec3 ====> dvec3
dvec3 var276 = ++var17; //dvec3
// u64vec4 ====> dvec4
dvec4 var277 = ++var23; //dvec4
// float ====> double
double var278 = ++var5; //double
// vec2 ====> dvec2
dvec2 var279 = ++var11; //dvec2
// vec3 ====> dvec3
dvec3 var280 = ++var17; //dvec3
// vec4 ====> dvec4
dvec4 var281 = ++var23; //dvec4
// float16_t ====> float, double
float var282 = ++var4; //float
double var283 = ++var5; //double
// f16vec2 ====> vec2, dvec2
vec2 var284 = ++var10; //vec2
dvec2 var285 = ++var11; //dvec2
// f16vec3 ====> vec3, dvec3
vec3 var286 = ++var16; //vec3
dvec3 var287 = ++var17; //dvec3
// f16vec4 ====> vec4, dvec4
vec4 var288 = ++var22; //vec4
dvec4 var289 = ++var23; //dvec4
// int ====> uint, int64_t, uint64_t, float, double
uint var290 = --var1; //uint
int64_t var291 = --var2; //int64_t
uint64_t var292 = --var3; //uint64_t
float var293 = --var4; //float
double var294 = --var5; //double
// ivec2 ====> uvec2, i64vec2, u64vec2, vec2, dvec2
uvec2 var295 = --var7; //uvec2
i64vec2 var296 = --var8; //i64vec2
u64vec2 var297 = --var9; //u64vec2
vec2 var298 = --var10; //vec2
dvec2 var299 = --var11; //dvec2
// ivec3 ====> uvec3, i64vec3, u64vec3, vec3, dvec3
uvec3 var300 = --var13; //uvec3
i64vec3 var301 = --var14; //i64vec3
u64vec3 var302 = --var15; //u64vec3
vec3 var303 = --var16; //vec3
dvec3 var304 = --var17; //dvec3
// ivec4 ====> uvec4, i64vec4, u64vec4, vec4, dvec4
uvec4 var305 = --var19; //uvec4
i64vec4 var306 = --var20; //i64vec4
u64vec4 var307 = --var21; //u64vec4
vec4 var308 = --var22; //vec4
dvec4 var309 = --var23; //dvec4
// int8_t ====> int, int64_t, uint, uint64_t, float, double
int var310 = --var0; //int
int64_t var311 = --var2; //int64_t
uint var312 = --var1; //uint
uint64_t var313 = --var3; //uint64_t
float var314 = --var4; //float
double var315 = --var5; //double
// int16_t ====> int, int64_t, uint, uint64_t, float, double
int var316 = --var0; //int
int64_t var317 = --var2; //int64_t
uint var318 = --var1; //uint
uint64_t var319 = --var3; //uint64_t
float var320 = --var4; //float
double var321 = --var5; //double
// i8vec2 ====> ivec2, i64vec2, uvec2, u64vec2, vec2, dvec2
ivec2 var322 = --var6; //ivec2
i64vec2 var323 = --var8; //i64vec2
uvec2 var324 = --var7; //uvec2
u64vec2 var325 = --var9; //u64vec2
vec2 var326 = --var10; //vec2
dvec2 var327 = --var11; //dvec2
// i16vec2 ====> ivec2, i64vec2, uvec2, u64vec2, vec2, dvec2
ivec2 var328 = --var6; //ivec2
i64vec2 var329 = --var8; //i64vec2
uvec2 var330 = --var7; //uvec2
u64vec2 var331 = --var9; //u64vec2
vec2 var332 = --var10; //vec2
dvec2 var333 = --var11; //dvec2
// i8vec3 ====> ivec3, i64vec3, uvec3, u64vec3, vec3, dvec3
ivec3 var334 = --var12; //ivec3
i64vec3 var335 = --var14; //i64vec3
uvec3 var336 = --var13; //uvec3
u64vec3 var337 = --var15; //u64vec3
vec3 var338 = --var16; //vec3
dvec3 var339 = --var17; //dvec3
// i16vec3 ====> uvec3, u64vec3, vec3, dvec3
uvec3 var340 = --var13; //uvec3
u64vec3 var341 = --var15; //u64vec3
vec3 var342 = --var16; //vec3
dvec3 var343 = --var17; //dvec3
// i8vec4 ====> ivec4, i64vec4, uvec4, u64vec4, vec4, dvec4
ivec4 var344 = --var18; //ivec4
i64vec4 var345 = --var20; //i64vec4
uvec4 var346 = --var19; //uvec4
u64vec4 var347 = --var21; //u64vec4
vec4 var348 = --var22; //vec4
dvec4 var349 = --var23; //dvec4
// i16vec4 ====> uvec4, u64vec4, vec4, dvec4
uvec4 var350 = --var19; //uvec4
u64vec4 var351 = --var21; //u64vec4
vec4 var352 = --var22; //vec4
dvec4 var353 = --var23; //dvec4
// int64_t ====> uint64_t, double
uint64_t var354 = --var3; //uint64_t
double var355 = --var5; //double
// i64vec2 ====> u64vec2, dvec2
u64vec2 var356 = --var9; //u64vec2
dvec2 var357 = --var11; //dvec2
// i64vec3 ====> u64vec3, dvec3
u64vec3 var358 = --var15; //u64vec3
dvec3 var359 = --var17; //dvec3
// i64vec4 ====> u64vec4, dvec4
u64vec4 var360 = --var21; //u64vec4
dvec4 var361 = --var23; //dvec4
// uint ====> uint64_t, float, double
uint64_t var362 = --var3; //uint64_t
float var363 = --var4; //float
double var364 = --var5; //double
// uvec2 ====> u64vec2, vec2, dvec2
u64vec2 var365 = --var9; //u64vec2
vec2 var366 = --var10; //vec2
dvec2 var367 = --var11; //dvec2
// uvec3 ====> u64vec3, vec3, dvec3
u64vec3 var368 = --var15; //u64vec3
vec3 var369 = --var16; //vec3
dvec3 var370 = --var17; //dvec3
// uvec4 ====> u64vec4, vec4, dvec4
u64vec4 var371 = --var21; //u64vec4
vec4 var372 = --var22; //vec4
dvec4 var373 = --var23; //dvec4
// uint8_t ====> uint, uint64_t, float, double
uint var374 = --var1; //uint
uint64_t var375 = --var3; //uint64_t
float var376 = --var4; //float
double var377 = --var5; //double
// uint16_t ====> uint, uint64_t, float, double
uint var378 = --var1; //uint
uint64_t var379 = --var3; //uint64_t
float var380 = --var4; //float
double var381 = --var5; //double
// u8vec2 ====> uvec2, u64vec2, vec2, dvec2
uvec2 var382 = --var7; //uvec2
u64vec2 var383 = --var9; //u64vec2
vec2 var384 = --var10; //vec2
dvec2 var385 = --var11; //dvec2
// u16vec2 ====> uvec2, u64vec2, vec2, dvec2
uvec2 var386 = --var7; //uvec2
u64vec2 var387 = --var9; //u64vec2
vec2 var388 = --var10; //vec2
dvec2 var389 = --var11; //dvec2
// u8vec3 ====> uvec3, u64vec3, vec3, dvec3
uvec3 var390 = --var13; //uvec3
u64vec3 var391 = --var15; //u64vec3
vec3 var392 = --var16; //vec3
dvec3 var393 = --var17; //dvec3
// u8vec4 ====> uvec4, u64vec4, vec4, dvec4
uvec4 var394 = --var19; //uvec4
u64vec4 var395 = --var21; //u64vec4
vec4 var396 = --var22; //vec4
dvec4 var397 = --var23; //dvec4
// uint64_t ====> double
double var398 = --var5; //double
// u64vec2 ====> dvec2
dvec2 var399 = --var11; //dvec2
// u64vec3 ====> dvec3
dvec3 var400 = --var17; //dvec3
// u64vec4 ====> dvec4
dvec4 var401 = --var23; //dvec4
// float ====> double
double var402 = --var5; //double
// vec2 ====> dvec2
dvec2 var403 = --var11; //dvec2
// vec3 ====> dvec3
dvec3 var404 = --var17; //dvec3
// vec4 ====> dvec4
dvec4 var405 = --var23; //dvec4
// float16_t ====> float, double
float var406 = --var4; //float
double var407 = --var5; //double
// f16vec2 ====> vec2, dvec2
vec2 var408 = --var10; //vec2
dvec2 var409 = --var11; //dvec2
// f16vec3 ====> vec3, dvec3
vec3 var410 = --var16; //vec3
dvec3 var411 = --var17; //dvec3
// f16vec4 ====> vec4, dvec4
vec4 var412 = --var22; //vec4
dvec4 var413 = --var23; //dvec4
}

void testConditionExpressions() {
int8_t    var1 = int8_t(0);
int16_t   var5 = int16_t(0);
int32_t   var9 = int32_t(0);
int64_t   var13 = int64_t(0);
float16_t var33 = float16_t(0);
float64_t var41 = float64_t(0);
double    var42 = double(0);

bool b = (var1 == int8_t(0));
b = (var5 == int16_t(0));
b = (var9 == int32_t(0));
b = (var13 == int64_t(0));
b = (var33 == float16_t(0));
b = (var41 == float64_t(0));
b = (var42 == double(0));

b = (var5 != int16_t(0));
b = (var9 != int32_t(0));
b = (var13 != int64_t(0));
b = (var33 != float16_t(0));
b = (var41 != float64_t(0));
b = (var42 != double(0));

b = (var5 > int16_t(0));
b = (var9 > int32_t(0));
b = (var13 > int64_t(0));
b = (var33 > float16_t(0));
b = (var41 > float64_t(0));
b = (var42 > double(0));

}


// This test case verifies all the function definition conversions as specificed in
// NV_gpu_shader5 under "Modify Section 6.1, Function Definitions, p. 63"

void func(int64_t a)
{
}

void func(int a)
{

}

void func(uint a)
{

}

void func(uint64_t a)
{

}

void func(float a) {

}

void func64(double a) {

}

void func64(int64_t a)
{

}

void func64(uint64_t a)
{

}

void testFunctionDefinition() {

int8_t    var1 = int8_t(0);
int16_t   var5 = int16_t(0);
uint8_t   var2 = uint8_t(0);
uint16_t  var3 = uint16_t(0);
int32_t   var9 = int32_t(0);
int       var10 = int(0);
uint      var11 = uint(0);
int64_t   var13 = int64_t(0);
float16_t var33 = float16_t(0);
float     var34 = float(0);
/*
 source types                destination types
-----------------           -----------------
int8_t, int16_t             int, int64_t
int                         int64_t
uint8_t, uint16_t           uint, uint64_t
uint                        uint64_t
float16_t                   float
float                       double
*/

func(var1); // should pick func(int a)
func(var5); // should pick func(int a)
func(var10); //int, should pick func(int64_t a)

func(var2); //uint8_t should pick func(uint a)
func(var3); //uint16_t should pick func(uint a)

func(var33); //float16_t should pick void func(float a)

func64(var1); // int8_t, should pick func64(int64_t a)
func64(var5); // int16_t, should pick func64(int64_t a)
func64(var9); // int32_t, should pick func64(int64_t a)

func64(var2); //uint8_t should pick func64(uint64_t a)
func64(var3); //uint16_t should pick func64(uint64_t a)
func64(var11); //uint should pick func64(uint64_t a)
func64(var34); //float should pick func64(double a)

}



flat in int var;
void testBuiltinNVOnly()
{
      // Test builtins defined by NV_gpu_shader5 and not available in ARB_gpu_shader5
      int64_t  v1 = packInt2x32(ivec2(0));
      uint64_t v2 = packUint2x32(uvec2(0));

      ivec2  v3 = unpackInt2x32(int64_t(0));
      uvec2  v4 = unpackUint2x32(uint64_t(0));
      uint   v5 = packFloat2x16(f16vec2(0));
      f16vec2 v6 =  unpackFloat2x16(uint(0));
      int64_t v7 = doubleBitsToInt64(double(0));
      i64vec2 v8 = doubleBitsToInt64(dvec2(0));
      i64vec3 v9 = doubleBitsToInt64(dvec3(0));
      i64vec4 v10 = doubleBitsToInt64(dvec4(0));

      uint64_t v11 = doubleBitsToUint64(double(0));
      u64vec2  v12 = doubleBitsToUint64(dvec2(0));
      u64vec3  v13 = doubleBitsToUint64(dvec3(0));
      u64vec4  v14 = doubleBitsToUint64(dvec4(0));

      double v15 = int64BitsToDouble(int64_t(0));
      dvec2  v16 = int64BitsToDouble(i64vec2(0));
      dvec3  v17 = int64BitsToDouble(i64vec3(0));
      dvec4  v18 = int64BitsToDouble(i64vec4(0));

      double v19 = uint64BitsToDouble(uint64_t(0));
      dvec2  v20 = uint64BitsToDouble(u64vec2(0));
      dvec3  v21 = uint64BitsToDouble(u64vec3(0));
      dvec4  v22 = uint64BitsToDouble(u64vec4(0));
      
      bool b1 = anyThreadNV((var == 1));      
      bool b2 = allThreadsNV((var == 1));      
      bool b3 = allThreadsEqualNV((var == 1));      

}


// Modifications to Vector Relational Functions
// Introduction of explicitly sized types

void testVectorRelationBuiltins()
{


    bvec2 b1 = lessThan(i64vec2(0), i64vec2(1));
    bvec3 b2 = lessThan(i64vec3(0), i64vec3(1));
    bvec4 b3 = lessThan(i64vec4(0), i64vec4(1));
    bvec2 b4 = lessThan(u64vec2(0), u64vec2(1));
    bvec3 b5 = lessThan(u64vec3(0), u64vec3(1));
    bvec4 b6 = lessThan(u64vec4(0), u64vec4(1));

    bvec2 b7 = lessThanEqual(i64vec2(0), i64vec2(1));
    bvec3 b8 = lessThanEqual(i64vec3(0), i64vec3(1));
    bvec4 b9 = lessThanEqual(i64vec4(0), i64vec4(1));
    bvec2 b10 = lessThanEqual(u64vec2(0), u64vec2(1));
    bvec3 b11 = lessThanEqual(u64vec3(0), u64vec3(1));
    bvec4 b12 = lessThanEqual(u64vec4(0), u64vec4(1));

    bvec2 b13 = greaterThan(i64vec2(0), i64vec2(1));
    bvec3 b14 = greaterThan(i64vec3(0), i64vec3(1));
    bvec4 b15 = greaterThan(i64vec4(0), i64vec4(1));
    bvec2 b16 = greaterThan(u64vec2(0), u64vec2(1));
    bvec3 b17 = greaterThan(u64vec3(0), u64vec3(1));
    bvec4 b18 = greaterThan(u64vec4(0), u64vec4(1));

    bvec2 b19 = greaterThanEqual(i64vec2(0), i64vec2(1));
    bvec3 b20 = greaterThanEqual(i64vec3(0), i64vec3(1));
    bvec4 b21 = greaterThanEqual(i64vec4(0), i64vec4(1));
    bvec2 b22 = greaterThanEqual(u64vec2(0), u64vec2(1));
    bvec3 b23 = greaterThanEqual(u64vec3(0), u64vec3(1));
    bvec4 b24 = greaterThanEqual(u64vec4(0), u64vec4(1));

    bvec2 b25 = equal(i64vec2(0), i64vec2(1));
    bvec3 b26 = equal(i64vec3(0), i64vec3(1));
    bvec4 b27 = equal(i64vec4(0), i64vec4(1));
    bvec2 b28 = equal(u64vec2(0), u64vec2(1));
    bvec3 b29 = equal(u64vec3(0), u64vec3(1));
    bvec4 b30 = equal(u64vec4(0), u64vec4(1));

    bvec2 b31 = notEqual(i64vec2(0), i64vec2(1));
    bvec3 b32 = notEqual(i64vec3(0), i64vec3(1));
    bvec4 b33 = notEqual(i64vec4(0), i64vec4(1));
    bvec2 b34 = notEqual(u64vec2(0), u64vec2(1));
    bvec3 b35 = notEqual(u64vec3(0), u64vec3(1));
    bvec4 b36 = notEqual(u64vec4(0), u64vec4(1));

    bvec2 b37 = lessThan(f16vec2(0), f16vec2(1));
    bvec3 b38 = lessThan(f16vec3(0), f16vec3(1));
    bvec4 b39 = lessThan(f16vec4(0), f16vec4(1));

    bvec2 b40 = lessThanEqual(f16vec2(0), f16vec2(1));
    bvec3 b41 = lessThanEqual(f16vec3(0), f16vec3(1));
    bvec4 b42 = lessThanEqual(f16vec4(0), f16vec4(1));

    bvec2 b43 = greaterThan(f16vec2(0), f16vec2(1));
    bvec3 b44 = greaterThan(f16vec3(0), f16vec3(1));
    bvec4 b45 = greaterThan(f16vec4(0), f16vec4(1));

    bvec2 b46 = greaterThanEqual(f16vec2(0), f16vec2(1));
    bvec3 b47 = greaterThanEqual(f16vec3(0), f16vec3(1));
    bvec4 b48 = greaterThanEqual(f16vec4(0), f16vec4(1));

    bvec2 b49 = equal(f16vec2(0), f16vec2(1));
    bvec3 b50 = equal(f16vec3(0), f16vec3(1));
    bvec4 b51 = equal(f16vec4(0), f16vec4(1));

    bvec2 b52 = notEqual(f16vec2(0), f16vec2(1));
    bvec3 b53 = notEqual(f16vec3(0), f16vec3(1));
    bvec4 b54 = notEqual(f16vec4(0), f16vec4(1));

    // Dependency on GL_ARB_gpu_shader_fp64
    bvec2 b55 = lessThan(dvec2(0), dvec2(1));
    bvec3 b56 = lessThan(dvec3(0), dvec3(1));
    bvec4 b57 = lessThan(dvec4(0), dvec4(1));

    bvec2 b58 = lessThanEqual(dvec2(0), dvec2(1));
    bvec3 b59 = lessThanEqual(dvec3(0), dvec3(1));
    bvec4 b60 = lessThanEqual(dvec4(0), dvec4(1));

    bvec2 b61 = greaterThan(dvec2(0), dvec2(1));
    bvec3 b62 = greaterThan(dvec3(0), dvec3(1));
    bvec4 b63 = greaterThan(dvec4(0), dvec4(1));

    bvec2 b64 = greaterThanEqual(dvec2(0), dvec2(1));
    bvec3 b65 = greaterThanEqual(dvec3(0), dvec3(1));
    bvec4 b66 = greaterThanEqual(dvec4(0), dvec4(1));

    bvec2 b67 = equal(dvec2(0), dvec2(1));
    bvec3 b68 = equal(dvec3(0), dvec3(1));
    bvec4 b69 = equal(dvec4(0), dvec4(1));

    bvec2 b70 = notEqual(dvec2(0), dvec2(1));
    bvec3 b71 = notEqual(dvec3(0), dvec3(1));
    bvec4 b72 = notEqual(dvec4(0), dvec4(1));

}


// Testing texture builtins with non constant "offset"
uniform sampler2DRectShadow tvar0;
uniform samplerCubeShadow tvar1;
uniform sampler1DArrayShadow tvar2;
uniform sampler1DShadow tvar3;
uniform sampler2DShadow tvar4;
uniform sampler2DArrayShadow tvar5;
uniform samplerCubeArrayShadow tvar6;
uniform isampler2DArray tvar7;
uniform usampler2DArray tvar8;
uniform sampler2DArray tvar9;
uniform usampler2D tvar10;
uniform sampler2D tvar11;
uniform isampler2D tvar12;
uniform isampler2DRect tvar13;
uniform sampler2DRect tvar14;
uniform usampler2DRect tvar15;
uniform usamplerCubeArray tvar16;
uniform isamplerCubeArray tvar17;
uniform samplerCubeArray tvar18;
uniform usampler3D tvar19;
uniform sampler3D tvar20;
uniform isampler3D tvar21;
uniform sampler1D tvar22;
uniform usampler1D tvar23;
uniform isampler1D tvar24;
uniform isamplerCube tvar25;
uniform samplerCube tvar26;
uniform usamplerCube tvar27;
uniform isampler1DArray tvar28;
uniform usampler1DArray tvar29;
uniform sampler1DArray tvar30;
in float tvar32;
flat in int tvar33;
in vec2 tvar37;
flat in ivec2 tvar38;
in vec3 tvar42;
flat in ivec3 tvar43;
in vec4 tvar81;



void main() {
vec4 tvar31 =textureOffset( tvar22, tvar32, tvar33, tvar32);
ivec4 tvar34 =textureOffset( tvar24, tvar32, tvar33, tvar32);
uvec4 tvar35 =textureOffset( tvar23, tvar32, tvar33, tvar32);
vec4 tvar36 =textureOffset( tvar11, tvar37, tvar38, tvar32);
ivec4 tvar39 =textureOffset( tvar12, tvar37, tvar38, tvar32);
uvec4 tvar40 =textureOffset( tvar10, tvar37, tvar38, tvar32);
vec4 tvar41 =textureOffset( tvar20, tvar42, tvar43, tvar32);
ivec4 tvar44 =textureOffset( tvar21, tvar42, tvar43, tvar32);
uvec4 tvar45 =textureOffset( tvar19, tvar42, tvar43, tvar32);
vec4 tvar46 =textureOffset( tvar14, tvar37, tvar38);
ivec4 tvar47 =textureOffset( tvar13, tvar37, tvar38);
uvec4 tvar48 =textureOffset( tvar15, tvar37, tvar38);
float tvar49 =textureOffset( tvar0, tvar42, tvar38);
float tvar50 =textureOffset( tvar3, tvar42, tvar33, tvar32);
float tvar51 =textureOffset( tvar4, tvar42, tvar38, tvar32);
vec4 tvar52 =textureOffset( tvar30, tvar37, tvar33, tvar32);
ivec4 tvar53 =textureOffset( tvar28, tvar37, tvar33, tvar32);
uvec4 tvar54 =textureOffset( tvar29, tvar37, tvar33, tvar32);
vec4 tvar55 =textureOffset( tvar9, tvar42, tvar38, tvar32);
ivec4 tvar56 =textureOffset( tvar7, tvar42, tvar38, tvar32);
uvec4 tvar57 =textureOffset( tvar8, tvar42, tvar38, tvar32);
float tvar58 =textureOffset( tvar2, tvar42, tvar33, tvar32);
vec4 tvar59 =texelFetchOffset( tvar22, tvar33, tvar33, tvar33);
ivec4 tvar60 =texelFetchOffset( tvar24, tvar33, tvar33, tvar33);
uvec4 tvar61 =texelFetchOffset( tvar23, tvar33, tvar33, tvar33);
vec4 tvar62 =texelFetchOffset( tvar11, tvar38, tvar33, tvar38);
ivec4 tvar63 =texelFetchOffset( tvar12, tvar38, tvar33, tvar38);
uvec4 tvar64 =texelFetchOffset( tvar10, tvar38, tvar33, tvar38);
vec4 tvar65 =texelFetchOffset( tvar20, tvar43, tvar33, tvar43);
ivec4 tvar66 =texelFetchOffset( tvar21, tvar43, tvar33, tvar43);
uvec4 tvar67 =texelFetchOffset( tvar19, tvar43, tvar33, tvar43);
vec4 tvar68 =texelFetchOffset( tvar14, tvar38, tvar38);
ivec4 tvar69 =texelFetchOffset( tvar13, tvar38, tvar38);
uvec4 tvar70 =texelFetchOffset( tvar15, tvar38, tvar38);
vec4 tvar71 =texelFetchOffset( tvar30, tvar38, tvar33, tvar33);
ivec4 tvar72 =texelFetchOffset( tvar28, tvar38, tvar33, tvar33);
uvec4 tvar73 =texelFetchOffset( tvar29, tvar38, tvar33, tvar33);
vec4 tvar74 =texelFetchOffset( tvar9, tvar43, tvar33, tvar38);
ivec4 tvar75 =texelFetchOffset( tvar7, tvar43, tvar33, tvar38);
uvec4 tvar76 =texelFetchOffset( tvar8, tvar43, tvar33, tvar38);
vec4 tvar77 =textureProjOffset( tvar22, tvar37, tvar33, tvar32);
ivec4 tvar78 =textureProjOffset( tvar24, tvar37, tvar33, tvar32);
uvec4 tvar79 =textureProjOffset( tvar23, tvar37, tvar33, tvar32);
vec4 tvar80 =textureProjOffset( tvar22, tvar81, tvar33, tvar32);
ivec4 tvar82 =textureProjOffset( tvar24, tvar81, tvar33, tvar32);
uvec4 tvar83 =textureProjOffset( tvar23, tvar81, tvar33, tvar32);
vec4 tvar84 =textureProjOffset( tvar11, tvar42, tvar38, tvar32);
ivec4 tvar85 =textureProjOffset( tvar12, tvar42, tvar38, tvar32);
uvec4 tvar86 =textureProjOffset( tvar10, tvar42, tvar38, tvar32);
vec4 tvar87 =textureProjOffset( tvar11, tvar81, tvar38, tvar32);
ivec4 tvar88 =textureProjOffset( tvar12, tvar81, tvar38, tvar32);
uvec4 tvar89 =textureProjOffset( tvar10, tvar81, tvar38, tvar32);
vec4 tvar90 =textureProjOffset( tvar20, tvar81, tvar43, tvar32);
ivec4 tvar91 =textureProjOffset( tvar21, tvar81, tvar43, tvar32);
uvec4 tvar92 =textureProjOffset( tvar19, tvar81, tvar43, tvar32);
vec4 tvar93 =textureProjOffset( tvar14, tvar42, tvar38);
ivec4 tvar94 =textureProjOffset( tvar13, tvar42, tvar38);
uvec4 tvar95 =textureProjOffset( tvar15, tvar42, tvar38);
vec4 tvar96 =textureProjOffset( tvar14, tvar81, tvar38);
ivec4 tvar97 =textureProjOffset( tvar13, tvar81, tvar38);
uvec4 tvar98 =textureProjOffset( tvar15, tvar81, tvar38);
float tvar99 =textureProjOffset( tvar0, tvar81, tvar38);
float tvar100 =textureProjOffset( tvar3, tvar81, tvar33, tvar32);
float tvar101 =textureProjOffset( tvar4, tvar81, tvar38, tvar32);
vec4 tvar102 =textureLodOffset( tvar22, tvar32, tvar32, tvar33);
ivec4 tvar103 =textureLodOffset( tvar24, tvar32, tvar32, tvar33);
uvec4 tvar104 =textureLodOffset( tvar23, tvar32, tvar32, tvar33);
vec4 tvar105 =textureLodOffset( tvar11, tvar37, tvar32, tvar38);
ivec4 tvar106 =textureLodOffset( tvar12, tvar37, tvar32, tvar38);
uvec4 tvar107 =textureLodOffset( tvar10, tvar37, tvar32, tvar38);
vec4 tvar108 =textureLodOffset( tvar20, tvar42, tvar32, tvar43);
ivec4 tvar109 =textureLodOffset( tvar21, tvar42, tvar32, tvar43);
uvec4 tvar110 =textureLodOffset( tvar19, tvar42, tvar32, tvar43);
float tvar111 =textureLodOffset( tvar3, tvar42, tvar32, tvar33);
float tvar112 =textureLodOffset( tvar4, tvar42, tvar32, tvar38);
vec4 tvar113 =textureLodOffset( tvar30, tvar37, tvar32, tvar33);
ivec4 tvar114 =textureLodOffset( tvar28, tvar37, tvar32, tvar33);
uvec4 tvar115 =textureLodOffset( tvar29, tvar37, tvar32, tvar33);
vec4 tvar116 =textureLodOffset( tvar9, tvar42, tvar32, tvar38);
ivec4 tvar117 =textureLodOffset( tvar7, tvar42, tvar32, tvar38);
uvec4 tvar118 =textureLodOffset( tvar8, tvar42, tvar32, tvar38);
float tvar119 =textureLodOffset( tvar2, tvar42, tvar32, tvar33);
vec4 tvar120 =textureProjLodOffset( tvar22, tvar37, tvar32, tvar33);
ivec4 tvar121 =textureProjLodOffset( tvar24, tvar37, tvar32, tvar33);
uvec4 tvar122 =textureProjLodOffset( tvar23, tvar37, tvar32, tvar33);
vec4 tvar123 =textureProjLodOffset( tvar22, tvar81, tvar32, tvar33);
ivec4 tvar124 =textureProjLodOffset( tvar24, tvar81, tvar32, tvar33);
uvec4 tvar125 =textureProjLodOffset( tvar23, tvar81, tvar32, tvar33);
vec4 tvar126 =textureProjLodOffset( tvar11, tvar42, tvar32, tvar38);
ivec4 tvar127 =textureProjLodOffset( tvar12, tvar42, tvar32, tvar38);
uvec4 tvar128 =textureProjLodOffset( tvar10, tvar42, tvar32, tvar38);
vec4 tvar129 =textureProjLodOffset( tvar11, tvar81, tvar32, tvar38);
ivec4 tvar130 =textureProjLodOffset( tvar12, tvar81, tvar32, tvar38);
uvec4 tvar131 =textureProjLodOffset( tvar10, tvar81, tvar32, tvar38);
vec4 tvar132 =textureProjLodOffset( tvar20, tvar81, tvar32, tvar43);
ivec4 tvar133 =textureProjLodOffset( tvar21, tvar81, tvar32, tvar43);
uvec4 tvar134 =textureProjLodOffset( tvar19, tvar81, tvar32, tvar43);
float tvar135 =textureProjLodOffset( tvar3, tvar81, tvar32, tvar33);
float tvar136 =textureProjLodOffset( tvar4, tvar81, tvar32, tvar38);
}
