// RUN: %dxc -E main -T ps_6_6 %s -DCT=float -DCC=1 -DSCALAR=1          | FileCheck %s -DELTY=F32   -DPROP1=265
// RUN: %dxc -E main -T ps_6_6 %s -DCT=float -DCC=1 -DSCALAR=1 -DSC=,0  | FileCheck %s -DELTY=F32   -DPROP1=265
// RUN: %dxc -E main -T ps_6_6 %s -DCT=float -DCC=1 -DSCALAR=1 -DSC=,8  | FileCheck %s -DELTY=F32   -DPROP1=524553
// RUN: %dxc -E main -T ps_6_6 %s -DCT=float -DCC=1                     | FileCheck %s -DELTY=F32   -DPROP1=265
// RUN: %dxc -E main -T ps_6_6 %s -DCT=float -DCC=2                     | FileCheck %s -DELTY=2xF32 -DPROP1=521
// RUN: %dxc -E main -T ps_6_6 %s -DCT=float -DCC=3                     | FileCheck %s -DELTY=3xF32 -DPROP1=777
// RUN: %dxc -E main -T ps_6_6 %s -DCT=float                            | FileCheck %s -DELTY=4xF32 -DPROP1=1033
// RUN: %dxc -E main -T ps_6_6 %s -DCT=float -DSC=,0                    | FileCheck %s -DELTY=4xF32 -DPROP1=1033
// RUN: %dxc -E main -T ps_6_6 %s -DCT=float -DSC=,8                    | FileCheck %s -DELTY=4xF32 -DPROP1=525321

// RUN: %dxc -E main -T ps_6_6 %s -DCT=int                              | FileCheck %s -DELTY=4xI32 -DPROP1=1028
// RUN: %dxc -E main -T ps_6_6 %s -DCT=uint                             | FileCheck %s -DELTY=4xU32 -DPROP1=1029

// half is float for shader type unless -enable-16bit-types is specified
// RUN: %dxc -E main -T ps_6_6 %s -DCT=half                             | FileCheck %s -DELTY=4xF32 -DPROP1=1033
// RUN: %dxc -E main -T ps_6_6 %s -DCT=half       -enable-16bit-types   | FileCheck %s -DELTY=4xF16 -DPROP1=1032

// component type is shader type, not storage type,
// so it's 16-bit for min-precision with or without -enable-16bit-types
// RUN: %dxc -E main -T ps_6_6 %s -DCT=min16float                       | FileCheck %s -DELTY=4xF16 -DPROP1=1032
// RUN: %dxc -E main -T ps_6_6 %s -DCT=min16float -enable-16bit-types   | FileCheck %s -DELTY=4xF16 -DPROP1=1032
// RUN: %dxc -E main -T ps_6_6 %s -DCT=min16int                         | FileCheck %s -DELTY=4xI16 -DPROP1=1026
// RUN: %dxc -E main -T ps_6_6 %s -DCT=min16int   -enable-16bit-types   | FileCheck %s -DELTY=4xI16 -DPROP1=1026

// Native 16-bit type looks the same in props as min16
// RUN: %dxc -E main -T ps_6_6 %s -DCT=float16_t  -enable-16bit-types   | FileCheck %s -DELTY=4xF16 -DPROP1=1032
// RUN: %dxc -E main -T ps_6_6 %s -DCT=int16_t    -enable-16bit-types   | FileCheck %s -DELTY=4xI16 -DPROP1=1026

// Ensure that MS textures from heap have expected properties

// CHECK: call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %{{.*}}, %dx.types.ResourceProperties { i32 3, i32 [[PROP1]] })
// CHECK-SAME: resource: Texture2DMS<[[ELTY]]>
// CHECK: call %dx.types.Handle @dx.op.annotateHandle(i32 216, %dx.types.Handle %{{.*}}, %dx.types.ResourceProperties { i32 8, i32 [[PROP1]] })
// CHECK-SAME: resource: Texture2DMSArray<[[ELTY]]>

// CT = ComponentType
// CC = ComponentCount
#ifndef CC
#define CC 4
#endif
// SC = SampleCount
#ifndef SC
#define SC
#endif

#ifdef SCALAR
Texture2DMS<CT SC> TexMS_scalar : register(t0);
Texture2DMSArray<CT SC> TexMSA_scalar : register(t1);
#else
Texture2DMS<vector<CT, CC> SC> TexMS_vector : register(t0);
Texture2DMSArray<vector<CT, CC> SC> TexMSA_vector : register(t1);
#endif

vector<CT, CC> main(int4 a : A, float4 coord : TEXCOORD) : SV_TARGET
{
  return (vector<CT, CC>)(0)
#ifdef SCALAR
    + TexMS_scalar.Load(a.xy, a.w)
    + TexMSA_scalar.Load(a.xyz, a.w)
#else
    + TexMS_vector.Load(a.xy, a.w)
    + TexMSA_vector.Load(a.xyz, a.w)
#endif
    ;
}
