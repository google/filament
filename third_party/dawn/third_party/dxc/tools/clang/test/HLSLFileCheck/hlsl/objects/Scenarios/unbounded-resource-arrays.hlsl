// RUN: %dxc -DDIM=1 -T ps_6_5 %s | FileCheck %s
// RUN: %dxc -DDIM=3 -T ps_6_5 %s | FileCheck %s
// RUN: %dxc -DDIM=3 -T ps_6_5 %s | FileCheck %s

// RUN: %dxc -DDIM=1 -T ps_6_6 %s | FileCheck %s
// RUN: %dxc -DDIM=2 -T ps_6_6 %s | FileCheck %s
// RUN: %dxc -DDIM=3 -T ps_6_6 %s | FileCheck %s

// Ensure that unbounded multidimensional resource arrays of various kinds
// will match both the legacy and 6.6 handle creation intrinsics

// Adjust subscript values so that the index is the same for all
#define BASE 3 // Just a base value from which to extract the interesting dimensions
#if DIM == 1
// For the single dimension, we only have the unbound axis, so the index is flattened here
#define SUB(ix) []
#define IDX(ix) [BASE*(BASE + 1 + ix) + 1]
#elif DIM == 2
// For 2x, the "row" width must be the size of the slice for 3x
// Then an index of [1][1] will result in a flat index of sliceSize + rowsize + 1
#define SUB(ix) [][BASE*(BASE + 1 + ix)]
#define IDX(ix) [1][1]
#else // DIM == 3
// For 3x, [1][1][1] will naturally result in sliceSize + rowSize + 1
#define SUB(ix) [][BASE + ix][BASE]
#define IDX(ix) [1][1][1]
#endif

struct MyFloat4 {
  float4 f;
};

// Declare resources with first dimension unbounded and
// second subscripts set to an interesting, unique value
Texture1D<float4>          Tex1D SUB(0) : register(t0, space0);
Texture1DArray<float4>     Tex1DArr SUB(1) : register(t0, space1);
Texture2D<float4>          Tex2D SUB(2) : register(t0, space2);
Texture2DArray<float4>     Tex2DArr SUB(3) : register(t0, space3);
Texture3D<float4>          Tex3D SUB(4) : register(t0, space4);

RWTexture1D<float4>        RWTex1D SUB(5) : register(u0, space5);
RWTexture1DArray<float4>   RWTex1DArr SUB(6) : register(u0, space6);
RWTexture2D<float4>        RWTex2D SUB(7) : register(u0, space7);
RWTexture2DArray<float4>   RWTex2DArr SUB(8) : register(u0, space8);
RWTexture3D<float4>        RWTex3D SUB(9) : register(u0, space9);

Texture2DMS<float4>        Tex2DMS SUB(10) : register(t0, space10);
Texture2DMSArray<float4>   Tex2DMSArr SUB(11) : register(t0, space11);
TextureCube<float4>        TexCube SUB(12) : register(t0, space12);
TextureCubeArray<float4>   TexCubeArr SUB(13) : register(t0, space13);
SamplerState               Samp SUB(14) : register(t0, space14);
SamplerComparisonState     SampCmp SUB(15) : register(t0, space15);


// ConstantBuffer and TextureBuffer get created first
// CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 64, i1 false)
// CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 61, i1 false)
ConstantBuffer<MyFloat4>   CBuf SUB(16) : register(b0, space16); // 1*(3+16)*3 + 1*3 + 1 = 61
TextureBuffer<MyFloat4>    TBuf SUB(17) : register(t0, space17); // 1*(3+17)*3 + 1*3 + 1 = 64

Buffer<float4>             Buf SUB(18) : register(t0, space18);
StructuredBuffer<float4>   SBuf SUB(19) : register(t0, space19);
ByteAddressBuffer          BABuf SUB(20) : register(t0, space20);

RWBuffer<float4>           RWBuf SUB(21) : register(u0, space21);
RWStructuredBuffer<float4> RWSBuf SUB(22) : register(u0, space22);
RWByteAddressBuffer        RWBABuf SUB(23) : register(u0, space23);

ConsumeStructuredBuffer<float4> CSBuf SUB(24) : register(u0, space24);
AppendStructuredBuffer<float4>  ASBuf SUB(25) : register(u0, space25);
FeedbackTexture2D<SAMPLER_FEEDBACK_MIN_MIP>      FBTex2D SUB(26) : register(u0, space26);
FeedbackTexture2DArray<SAMPLER_FEEDBACK_MIN_MIP> FBTex2DArr SUB(27) : register(u0, space27);

float4 main(int4 a : A, float4 f : F) : SV_Target {

  // CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 13, i1 false)
  // CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 16, i1 false)
  // CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 19, i1 false)
  // CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 22, i1 false)
  // CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 25, i1 false)
  float4 res = Tex1D IDX(0).Load(a.x) // 1*(3+0)*3 + 1*3 + 1 = 13
  * Tex1DArr IDX(1).Load(a.xyz)       // 1*(3+1)*3 + 1*3 + 1 = 16
  * Tex2D IDX(2).Load(a.xyz)          // 1*(3+2)*3 + 1*3 + 1 = 19
  * Tex2DArr IDX(3).Load(a.xyzw)      // 1*(3+3)*3 + 1*3 + 1 = 22
  * Tex3D IDX(4).Load(a.xyzw)         // 1*(3+4)*3 + 1*3 + 1 = 25

  // CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 28, i1 false)
  // CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 31, i1 false)
  // CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 34, i1 false)
  // CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 37, i1 false)
  // CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 40, i1 false)
  * RWTex1D IDX(5).Load(a.x)          // 1*(3+5)*3 + 1*3 + 1 = 28
  * RWTex1DArr IDX(6).Load(a.xy)      // 1*(3+6)*3 + 1*3 + 1 = 31
  * RWTex2D IDX(7).Load(a.xy)         // 1*(3+7)*3 + 1*3 + 1 = 34
  * RWTex2DArr IDX(8).Load(a.xyz)     // 1*(3+8)*3 + 1*3 + 1 = 37
  * RWTex3D IDX(9).Load(a.xyz)        // 1*(3+9)*3 + 1*3 + 1 = 40

  // CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 43, i1 false)
  // CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 46, i1 false)
  // CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 49, i1 false)
  // CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 55, i1 false)
  // CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 52, i1 false)
  // CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 58, i1 false)
  * Tex2DMS IDX(10).Load(a.xy, a.w)              // 1*(3+10)*3 + 1*3 + 1 = 43
  * Tex2DMSArr IDX(11).Load(a.xyz, a.w)          // 1*(3+11)*3 + 1*3 + 1 = 46
  * TexCube IDX(12).Sample(Samp IDX(14), f.xyz)  // 1*(3+12)*3 + 1*3 + 1 = 49; 1*(3+14)*3 + 1*3 + 1 = 55
  * TexCubeArr IDX(13).SampleCmp(SampCmp IDX(15), f.xyzw, 0.0) // 1*(3+13)*3 + 1*3 + 1 = 52; 1*(3+15)*3 + 1*3 + 1 = 58

  // CHECKS at the top
  * CBuf IDX(16).f
  * TBuf IDX(17).f

  // CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 67, i1 false)
  // CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 70, i1 false)
  // CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 73, i1 false)
  * Buf IDX(18).Load(a.x)            // 1*(3+18)*3 + 1*3 + 1 = 67
  * SBuf IDX(19).Load(a.x)           // 1*(3+19)*3 + 1*3 + 1 = 70
  * BABuf IDX(20).Load<float4>(a.x)  // 1*(3+20)*3 + 1*3 + 1 = 73

  // CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 76, i1 false)
  // CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 79, i1 false)
  // CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 82, i1 false)
  // CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 85, i1 false)
  * RWBuf IDX(21).Load(a.x)          // 1*(3+21)*3 + 1*3 + 1 = 76
  * RWSBuf IDX(22).Load(a.x)         // 1*(3+22)*3 + 1*3 + 1 = 79
  * RWBABuf IDX(23).Load<float4>(a.x)// 1*(3+23)*3 + 1*3 + 1 = 82
  * CSBuf IDX(24).Consume();         // 1*(3+24)*3 + 1*3 + 1 = 85

  // CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 88, i1 false)
  // CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 91, i1 false)
  // CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 19, i1 false)
  // CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 55, i1 false)
  // CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 94, i1 false)
  // CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 22, i1 false)
  // CHECK: call %dx.types.Handle @dx.op.createHandle{{.*}}i32 55, i1 false)
  ASBuf IDX(25).Append(3.0);         // 1*(3+25)*3 + 1*3 + 1 = 88
  FBTex2D IDX(26).WriteSamplerFeedback(Tex2D IDX(2), Samp IDX(14), f.xy); // 1*(3+26)*3 + 1*3 + 1 = 91; 1*(3+2)*3 + 1*3 + 1 = 19; 1*(3+14)*3 + 1*3 + 1 = 55
  FBTex2DArr IDX(27).WriteSamplerFeedback(Tex2DArr IDX(3), Samp IDX(14), f.xyz); // 1*(3+27)*3 + 1*3 + 1 = 94; 1*(3+3)*3 + 1*3 + 1 = 22; 1*(3+14)*3 + 1*3 + 1 = 55

  return res;
}
