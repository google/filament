// This file contains tests covering all overloads of mul intrinsic
// as documented here: https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-mul

// TODO: While testing overloads of mul() intrinsics, found some incorrect codegen for bool type.
// Add coverage for bool type once the issue #2467 is fixed.

// *****************************
// float overloads
// *****************************

// vectors and scalars
// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=float4 -DELEM_TY2=float4 -DRET_TY=float %s  | FileCheck %s -check-prefix=FL4_OVRLD
// FL4_OVRLD: call float @dx.op.dot4.f32

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=float3 -DELEM_TY2=float3 -DRET_TY=float %s  | FileCheck %s -check-prefix=FL3_OVRLD
// FL3_OVRLD: call float @dx.op.dot3.f32

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=float2 -DELEM_TY2=float2 -DRET_TY=float %s  | FileCheck %s -check-prefix=FL2_OVRLD
// FL2_OVRLD: call float @dx.op.dot2.f32

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=float -DELEM_TY2=float -DRET_TY=float %s  | FileCheck %s -check-prefix=FL_OVRLD
// FL_OVRLD: fmul fast float

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=float4 -DELEM_TY2=float4 -DRET_TY=float %s  | FileCheck %s -check-prefix=FL4_OVRLD_OD
// FL4_OVRLD_OD: call float @dx.op.dot4.f32

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=float3 -DELEM_TY2=float3 -DRET_TY=float %s  | FileCheck %s -check-prefix=FL3_OVRLD_OD
// FL3_OVRLD_OD: call float @dx.op.dot3.f32

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=float2 -DELEM_TY2=float2 -DRET_TY=float %s  | FileCheck %s -check-prefix=FL2_OVRLD_OD
// FL2_OVRLD_OD: call float @dx.op.dot2.f32

// matrix
// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=float2x4 -DELEM_TY2=float4x3 -DRET_TY=float2x3 %s  | FileCheck %s -check-prefix=FLMAT1_OVRLD
// FLMAT1_OVRLD: call float @dx.op.tertiary.f32(i32 46
// FLMAT1_OVRLD: call float @dx.op.tertiary.f32(i32 46
// FLMAT1_OVRLD: call float @dx.op.tertiary.f32(i32 46
// FLMAT1_OVRLD: fmul fast float

// FLMAT1_OVRLD: call float @dx.op.tertiary.f32(i32 46
// FLMAT1_OVRLD: call float @dx.op.tertiary.f32(i32 46
// FLMAT1_OVRLD: call float @dx.op.tertiary.f32(i32 46
// FLMAT1_OVRLD: fmul fast float

// FLMAT1_OVRLD: call float @dx.op.tertiary.f32(i32 46
// FLMAT1_OVRLD: call float @dx.op.tertiary.f32(i32 46
// FLMAT1_OVRLD: call float @dx.op.tertiary.f32(i32 46
// FLMAT1_OVRLD: fmul fast float

// FLMAT1_OVRLD: call float @dx.op.tertiary.f32(i32 46
// FLMAT1_OVRLD: call float @dx.op.tertiary.f32(i32 46
// FLMAT1_OVRLD: call float @dx.op.tertiary.f32(i32 46
// FLMAT1_OVRLD: fmul fast float

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=float1x4 -DELEM_TY2=float4x1 -DRET_TY=float1x1 %s  | FileCheck %s -check-prefix=FLMAT2_OVRLD
// FLMAT2_OVRLD: fmul fast float
// FLMAT2_OVRLD: call float @dx.op.tertiary.f32(i32 46
// FLMAT2_OVRLD: call float @dx.op.tertiary.f32(i32 46
// FLMAT2_OVRLD: call float @dx.op.tertiary.f32(i32 46

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=float2x4 -DELEM_TY2=float4x3 -DRET_TY=float2x3 %s  | FileCheck %s -check-prefix=FLMAT1_OVRLD_OD
// FLMAT1_OVRLD_OD: call float @dx.op.tertiary.f32(i32 46
// FLMAT1_OVRLD_OD: call float @dx.op.tertiary.f32(i32 46
// FLMAT1_OVRLD_OD: call float @dx.op.tertiary.f32(i32 46
// FLMAT1_OVRLD_OD: fmul fast float

// FLMAT1_OVRLD_OD: call float @dx.op.tertiary.f32(i32 46
// FLMAT1_OVRLD_OD: call float @dx.op.tertiary.f32(i32 46
// FLMAT1_OVRLD_OD: call float @dx.op.tertiary.f32(i32 46
// FLMAT1_OVRLD_OD: fmul fast float

// FLMAT1_OVRLD_OD: call float @dx.op.tertiary.f32(i32 46
// FLMAT1_OVRLD_OD: call float @dx.op.tertiary.f32(i32 46
// FLMAT1_OVRLD_OD: call float @dx.op.tertiary.f32(i32 46
// FLMAT1_OVRLD_OD: fmul fast float

// FLMAT1_OVRLD_OD: call float @dx.op.tertiary.f32(i32 46
// FLMAT1_OVRLD_OD: call float @dx.op.tertiary.f32(i32 46
// FLMAT1_OVRLD_OD: call float @dx.op.tertiary.f32(i32 46
// FLMAT1_OVRLD_OD: fmul fast float

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=float1x4 -DELEM_TY2=float4x1 -DRET_TY=float1x1 %s  | FileCheck %s -check-prefix=FLMAT2_OVRLD_OD
// FLMAT2_OVRLD_OD: fmul fast float
// FLMAT2_OVRLD_OD: call float @dx.op.tertiary.f32(i32 46
// FLMAT2_OVRLD_OD: call float @dx.op.tertiary.f32(i32 46
// FLMAT2_OVRLD_OD: call float @dx.op.tertiary.f32(i32 46

// mixed: scalar and vector
// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=float -DELEM_TY2=float4 -DRET_TY=float4 %s  | FileCheck %s -check-prefix=FL1_4_OVRLD
// FL1_4_OVRLD: fmul fast float
// FL1_4_OVRLD: fmul fast float
// FL1_4_OVRLD: fmul fast float
// FL1_4_OVRLD: fmul fast float

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=float3 -DELEM_TY2=float -DRET_TY=float3 %s  | FileCheck %s -check-prefix=FL3_1_OVRLD
// FL3_1_OVRLD: fmul fast float
// FL3_1_OVRLD: fmul fast float
// FL3_1_OVRLD: fmul fast float

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=float -DELEM_TY2=float4 -DRET_TY=float4 %s  | FileCheck %s -check-prefix=FL1_4_OVRLD_OD
// FL1_4_OVRLD_OD: fmul fast float
// FL1_4_OVRLD_OD: fmul fast float
// FL1_4_OVRLD_OD: fmul fast float
// FL1_4_OVRLD_OD: fmul fast float

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=float3 -DELEM_TY2=float -DRET_TY=float3 %s  | FileCheck %s -check-prefix=FL3_1_OVRLD_OD
// FL3_1_OVRLD_OD: fmul fast float
// FL3_1_OVRLD_OD: fmul fast float
// FL3_1_OVRLD_OD: fmul fast float

// mixed: scalar and matrix
// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=float -DELEM_TY2=float2x4 -DRET_TY=float2x4 %s  | FileCheck %s -check-prefix=FL1_MAT1_OVRLD
// FL1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT1_OVRLD: fmul fast float
// FL1_MAT1_OVRLD: fmul fast float
// FL1_MAT1_OVRLD: fmul fast float
// FL1_MAT1_OVRLD: fmul fast float
// FL1_MAT1_OVRLD: fmul fast float
// FL1_MAT1_OVRLD: fmul fast float
// FL1_MAT1_OVRLD: fmul fast float
// FL1_MAT1_OVRLD: fmul fast float

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=float4x3 -DELEM_TY2=float -DRET_TY=float4x3 %s  | FileCheck %s -check-prefix=FL1_MAT2_OVRLD
// FL1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT2_OVRLD: fmul fast float
// FL1_MAT2_OVRLD: fmul fast float
// FL1_MAT2_OVRLD: fmul fast float
// FL1_MAT2_OVRLD: fmul fast float
// FL1_MAT2_OVRLD: fmul fast float
// FL1_MAT2_OVRLD: fmul fast float
// FL1_MAT2_OVRLD: fmul fast float
// FL1_MAT2_OVRLD: fmul fast float
// FL1_MAT2_OVRLD: fmul fast float
// FL1_MAT2_OVRLD: fmul fast float
// FL1_MAT2_OVRLD: fmul fast float
// FL1_MAT2_OVRLD: fmul fast float

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=float -DELEM_TY2=float2x4 -DRET_TY=float2x4 %s  | FileCheck %s -check-prefix=FL1_MAT1_OVRLD_OD
// FL1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT1_OVRLD_OD: fmul fast float
// FL1_MAT1_OVRLD_OD: fmul fast float
// FL1_MAT1_OVRLD_OD: fmul fast float
// FL1_MAT1_OVRLD_OD: fmul fast float
// FL1_MAT1_OVRLD_OD: fmul fast float
// FL1_MAT1_OVRLD_OD: fmul fast float
// FL1_MAT1_OVRLD_OD: fmul fast float
// FL1_MAT1_OVRLD_OD: fmul fast float

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=float4x3 -DELEM_TY2=float -DRET_TY=float4x3 %s  | FileCheck %s -check-prefix=FL1_MAT2_OVRLD_OD
// FL1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.f32
// FL1_MAT2_OVRLD_OD: fmul fast float
// FL1_MAT2_OVRLD_OD: fmul fast float
// FL1_MAT2_OVRLD_OD: fmul fast float
// FL1_MAT2_OVRLD_OD: fmul fast float
// FL1_MAT2_OVRLD_OD: fmul fast float
// FL1_MAT2_OVRLD_OD: fmul fast float
// FL1_MAT2_OVRLD_OD: fmul fast float
// FL1_MAT2_OVRLD_OD: fmul fast float
// FL1_MAT2_OVRLD_OD: fmul fast float
// FL1_MAT2_OVRLD_OD: fmul fast float
// FL1_MAT2_OVRLD_OD: fmul fast float
// FL1_MAT2_OVRLD_OD: fmul fast float

// mixed: vector and matrix
// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=float2 -DELEM_TY2=float2x4 -DRET_TY=float4 %s  | FileCheck %s -check-prefix=FL2_MAT1_OVRLD
// FL2_MAT1_OVRLD: fmul fast float
// FL2_MAT1_OVRLD: call float @dx.op.tertiary.f32(i32 46
// FL2_MAT1_OVRLD: fmul fast float
// FL2_MAT1_OVRLD: call float @dx.op.tertiary.f32(i32 46
// FL2_MAT1_OVRLD: fmul fast float
// FL2_MAT1_OVRLD: call float @dx.op.tertiary.f32(i32 46
// FL2_MAT1_OVRLD: fmul fast float

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=float4x3 -DELEM_TY2=float3 -DRET_TY=float4 %s  | FileCheck %s -check-prefix=FL3_MAT2_OVRLD
// FL3_MAT2_OVRLD: fmul fast float
// FL3_MAT2_OVRLD: call float @dx.op.tertiary.f32(i32 46
// FL3_MAT2_OVRLD: call float @dx.op.tertiary.f32(i32 46
// FL3_MAT2_OVRLD: fmul fast float
// FL3_MAT2_OVRLD: call float @dx.op.tertiary.f32(i32 46
// FL3_MAT2_OVRLD: call float @dx.op.tertiary.f32(i32 46
// FL3_MAT2_OVRLD: fmul fast float
// FL3_MAT2_OVRLD: call float @dx.op.tertiary.f32(i32 46
// FL3_MAT2_OVRLD: call float @dx.op.tertiary.f32(i32 46
// FL3_MAT2_OVRLD: fmul fast float
// FL3_MAT2_OVRLD: call float @dx.op.tertiary.f32(i32 46
// FL3_MAT2_OVRLD: call float @dx.op.tertiary.f32(i32 46

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=float2 -DELEM_TY2=float2x4 -DRET_TY=float4 %s  | FileCheck %s -check-prefix=FL2_MAT1_OVRLD_OD
// FL2_MAT1_OVRLD_OD: fmul fast float
// FL2_MAT1_OVRLD_OD: call float @dx.op.tertiary.f32(i32 46
// FL2_MAT1_OVRLD_OD: fmul fast float
// FL2_MAT1_OVRLD_OD: call float @dx.op.tertiary.f32(i32 46
// FL2_MAT1_OVRLD_OD: fmul fast float
// FL2_MAT1_OVRLD_OD: call float @dx.op.tertiary.f32(i32 46
// FL2_MAT1_OVRLD_OD: fmul fast float

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=float4x3 -DELEM_TY2=float3 -DRET_TY=float4 %s  | FileCheck %s -check-prefix=FL3_MAT2_OVRLD_OD
// FL3_MAT2_OVRLD_OD: fmul fast float
// FL3_MAT2_OVRLD_OD: call float @dx.op.tertiary.f32(i32 46
// FL3_MAT2_OVRLD_OD: call float @dx.op.tertiary.f32(i32 46
// FL3_MAT2_OVRLD_OD: fmul fast float
// FL3_MAT2_OVRLD_OD: call float @dx.op.tertiary.f32(i32 46
// FL3_MAT2_OVRLD_OD: call float @dx.op.tertiary.f32(i32 46
// FL3_MAT2_OVRLD_OD: fmul fast float
// FL3_MAT2_OVRLD_OD: call float @dx.op.tertiary.f32(i32 46
// FL3_MAT2_OVRLD_OD: call float @dx.op.tertiary.f32(i32 46
// FL3_MAT2_OVRLD_OD: fmul fast float
// FL3_MAT2_OVRLD_OD: call float @dx.op.tertiary.f32(i32 46
// FL3_MAT2_OVRLD_OD: call float @dx.op.tertiary.f32(i32 46

// *****************************
// int overloads
// *****************************

// vectors and scalars
// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=int4 -DELEM_TY2=int4 -DRET_TY=int %s  | FileCheck %s -check-prefix=IN4_OVRLD
// IN4_OVRLD: call i32 @dx.op.tertiary.i32(i32 48

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=int3 -DELEM_TY2=int3 -DRET_TY=int %s  | FileCheck %s -check-prefix=IN3_OVRLD
// IN3_OVRLD: call i32 @dx.op.tertiary.i32(i32 48

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=int2 -DELEM_TY2=int2 -DRET_TY=int %s  | FileCheck %s -check-prefix=IN2_OVRLD
// IN2_OVRLD: call i32 @dx.op.tertiary.i32(i32 48

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=int -DELEM_TY2=int -DRET_TY=int %s  | FileCheck %s -check-prefix=IN_OVRLD
// IN_OVRLD: mul i32

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=int4 -DELEM_TY2=int4 -DRET_TY=int %s  | FileCheck %s -check-prefix=IN4_OVRLD_OD
// IN4_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 48

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=int3 -DELEM_TY2=int3 -DRET_TY=int %s  | FileCheck %s -check-prefix=IN3_OVRLD_OD
// IN3_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 48

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=int2 -DELEM_TY2=int2 -DRET_TY=int %s  | FileCheck %s -check-prefix=IN2_OVRLD_OD
// IN2_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 48

// matrix
// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=int2x4 -DELEM_TY2=int4x3 -DRET_TY=int2x3 %s  | FileCheck %s -check-prefix=INMAT1_OVRLD
// INMAT1_OVRLD: call i32 @dx.op.tertiary.i32(i32 48
// INMAT1_OVRLD: call i32 @dx.op.tertiary.i32(i32 48
// INMAT1_OVRLD: call i32 @dx.op.tertiary.i32(i32 48
// INMAT1_OVRLD: mul i32

// INMAT1_OVRLD: call i32 @dx.op.tertiary.i32(i32 48
// INMAT1_OVRLD: call i32 @dx.op.tertiary.i32(i32 48
// INMAT1_OVRLD: call i32 @dx.op.tertiary.i32(i32 48
// INMAT1_OVRLD: mul i32

// INMAT1_OVRLD: call i32 @dx.op.tertiary.i32(i32 48
// INMAT1_OVRLD: call i32 @dx.op.tertiary.i32(i32 48
// INMAT1_OVRLD: call i32 @dx.op.tertiary.i32(i32 48
// INMAT1_OVRLD: mul i32

// INMAT1_OVRLD: call i32 @dx.op.tertiary.i32(i32 48
// INMAT1_OVRLD: call i32 @dx.op.tertiary.i32(i32 48
// INMAT1_OVRLD: call i32 @dx.op.tertiary.i32(i32 48
// INMAT1_OVRLD: mul i32

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=int1x4 -DELEM_TY2=int4x1 -DRET_TY=int1x1 %s  | FileCheck %s -check-prefix=INMAT2_OVRLD
// INMAT2_OVRLD: mul i32
// INMAT2_OVRLD: call i32 @dx.op.tertiary.i32(i32 48
// INMAT2_OVRLD: call i32 @dx.op.tertiary.i32(i32 48
// INMAT2_OVRLD: call i32 @dx.op.tertiary.i32(i32 48

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=int2x4 -DELEM_TY2=int4x3 -DRET_TY=int2x3 %s  | FileCheck %s -check-prefix=INMAT1_OVRLD_OD
// INMAT1_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 48
// INMAT1_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 48
// INMAT1_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 48
// INMAT1_OVRLD_OD: mul i32

// INMAT1_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 48
// INMAT1_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 48
// INMAT1_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 48
// INMAT1_OVRLD_OD: mul i32

// INMAT1_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 48
// INMAT1_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 48
// INMAT1_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 48
// INMAT1_OVRLD_OD: mul i32

// INMAT1_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 48
// INMAT1_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 48
// INMAT1_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 48
// INMAT1_OVRLD_OD: mul i32

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=int1x4 -DELEM_TY2=int4x1 -DRET_TY=int1x1 %s  | FileCheck %s -check-prefix=INMAT2_OVRLD_OD
// INMAT2_OVRLD_OD: mul i32
// INMAT2_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 48
// INMAT2_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 48
// INMAT2_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 48

// mixed: scalar and vector
// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=int -DELEM_TY2=int4 -DRET_TY=int4 %s  | FileCheck %s -check-prefix=IN1_4_OVRLD
// IN1_4_OVRLD: mul i32
// IN1_4_OVRLD: mul i32
// IN1_4_OVRLD: mul i32
// IN1_4_OVRLD: mul i32

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=int3 -DELEM_TY2=int -DRET_TY=int3 %s  | FileCheck %s -check-prefix=IN3_1_OVRLD
// IN3_1_OVRLD: mul i32
// IN3_1_OVRLD: mul i32
// IN3_1_OVRLD: mul i32

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=int -DELEM_TY2=int4 -DRET_TY=int4 %s  | FileCheck %s -check-prefix=IN1_4_OVRLD_OD
// IN1_4_OVRLD_OD: mul i32
// IN1_4_OVRLD_OD: mul i32
// IN1_4_OVRLD_OD: mul i32
// IN1_4_OVRLD_OD: mul i32

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=int3 -DELEM_TY2=int -DRET_TY=int3 %s  | FileCheck %s -check-prefix=IN3_1_OVRLD_OD
// IN3_1_OVRLD_OD: mul i32
// IN3_1_OVRLD_OD: mul i32
// IN3_1_OVRLD_OD: mul i32

// mixed: scalar and matrix
// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=int -DELEM_TY2=int2x4 -DRET_TY=int2x4 %s  | FileCheck %s -check-prefix=IN1_MAT1_OVRLD
// IN1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT1_OVRLD: mul i32
// IN1_MAT1_OVRLD: mul i32
// IN1_MAT1_OVRLD: mul i32
// IN1_MAT1_OVRLD: mul i32
// IN1_MAT1_OVRLD: mul i32
// IN1_MAT1_OVRLD: mul i32
// IN1_MAT1_OVRLD: mul i32
// IN1_MAT1_OVRLD: mul i32

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=int4x3 -DELEM_TY2=int -DRET_TY=int4x3 %s  | FileCheck %s -check-prefix=IN1_MAT2_OVRLD
// IN1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT2_OVRLD: mul i32
// IN1_MAT2_OVRLD: mul i32
// IN1_MAT2_OVRLD: mul i32
// IN1_MAT2_OVRLD: mul i32
// IN1_MAT2_OVRLD: mul i32
// IN1_MAT2_OVRLD: mul i32
// IN1_MAT2_OVRLD: mul i32
// IN1_MAT2_OVRLD: mul i32
// IN1_MAT2_OVRLD: mul i32
// IN1_MAT2_OVRLD: mul i32
// IN1_MAT2_OVRLD: mul i32
// IN1_MAT2_OVRLD: mul i32

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=int -DELEM_TY2=int2x4 -DRET_TY=int2x4 %s  | FileCheck %s -check-prefix=IN1_MAT1_OVRLD_OD
// IN1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT1_OVRLD_OD: mul i32
// IN1_MAT1_OVRLD_OD: mul i32
// IN1_MAT1_OVRLD_OD: mul i32
// IN1_MAT1_OVRLD_OD: mul i32
// IN1_MAT1_OVRLD_OD: mul i32
// IN1_MAT1_OVRLD_OD: mul i32
// IN1_MAT1_OVRLD_OD: mul i32
// IN1_MAT1_OVRLD_OD: mul i32

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=int4x3 -DELEM_TY2=int -DRET_TY=int4x3 %s  | FileCheck %s -check-prefix=IN1_MAT2_OVRLD_OD
// IN1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// IN1_MAT2_OVRLD_OD: mul i32
// IN1_MAT2_OVRLD_OD: mul i32
// IN1_MAT2_OVRLD_OD: mul i32
// IN1_MAT2_OVRLD_OD: mul i32
// IN1_MAT2_OVRLD_OD: mul i32
// IN1_MAT2_OVRLD_OD: mul i32
// IN1_MAT2_OVRLD_OD: mul i32
// IN1_MAT2_OVRLD_OD: mul i32
// IN1_MAT2_OVRLD_OD: mul i32
// IN1_MAT2_OVRLD_OD: mul i32
// IN1_MAT2_OVRLD_OD: mul i32
// IN1_MAT2_OVRLD_OD: mul i32

// mixed: vector and matrix
// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=int2 -DELEM_TY2=int2x4 -DRET_TY=int4 %s  | FileCheck %s -check-prefix=IN2_MAT1_OVRLD
// IN2_MAT1_OVRLD: mul i32
// IN2_MAT1_OVRLD: call i32 @dx.op.tertiary.i32(i32 48
// IN2_MAT1_OVRLD: mul i32
// IN2_MAT1_OVRLD: call i32 @dx.op.tertiary.i32(i32 48
// IN2_MAT1_OVRLD: mul i32
// IN2_MAT1_OVRLD: call i32 @dx.op.tertiary.i32(i32 48
// IN2_MAT1_OVRLD: mul i32

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=int4x3 -DELEM_TY2=int3 -DRET_TY=int4 %s  | FileCheck %s -check-prefix=IN3_MAT2_OVRLD
// IN3_MAT2_OVRLD: mul i32
// IN3_MAT2_OVRLD: call i32 @dx.op.tertiary.i32(i32 48
// IN3_MAT2_OVRLD: call i32 @dx.op.tertiary.i32(i32 48
// IN3_MAT2_OVRLD: mul i32
// IN3_MAT2_OVRLD: call i32 @dx.op.tertiary.i32(i32 48
// IN3_MAT2_OVRLD: call i32 @dx.op.tertiary.i32(i32 48
// IN3_MAT2_OVRLD: mul i32
// IN3_MAT2_OVRLD: call i32 @dx.op.tertiary.i32(i32 48
// IN3_MAT2_OVRLD: call i32 @dx.op.tertiary.i32(i32 48
// IN3_MAT2_OVRLD: mul i32
// IN3_MAT2_OVRLD: call i32 @dx.op.tertiary.i32(i32 48
// IN3_MAT2_OVRLD: call i32 @dx.op.tertiary.i32(i32 48

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=int2 -DELEM_TY2=int2x4 -DRET_TY=int4 %s  | FileCheck %s -check-prefix=IN2_MAT1_OVRLD_OD
// IN2_MAT1_OVRLD_OD: mul i32
// IN2_MAT1_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 48
// IN2_MAT1_OVRLD_OD: mul i32
// IN2_MAT1_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 48
// IN2_MAT1_OVRLD_OD: mul i32
// IN2_MAT1_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 48
// IN2_MAT1_OVRLD_OD: mul i32

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=int4x3 -DELEM_TY2=int3 -DRET_TY=int4 %s  | FileCheck %s -check-prefix=IN3_MAT2_OVRLD_OD
// IN3_MAT2_OVRLD_OD: mul i32
// IN3_MAT2_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 48
// IN3_MAT2_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 48
// IN3_MAT2_OVRLD_OD: mul i32
// IN3_MAT2_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 48
// IN3_MAT2_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 48
// IN3_MAT2_OVRLD_OD: mul i32
// IN3_MAT2_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 48
// IN3_MAT2_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 48
// IN3_MAT2_OVRLD_OD: mul i32
// IN3_MAT2_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 48
// IN3_MAT2_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 48

// *****************************
// uint overloads
// *****************************

// vectors and scalars
// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=uint4 -DELEM_TY2=uint4 -DRET_TY=uint %s  | FileCheck %s -check-prefix=UIN4_OVRLD
// UIN4_OVRLD: call i32 @dx.op.tertiary.i32(i32 49

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=uint3 -DELEM_TY2=uint3 -DRET_TY=uint %s  | FileCheck %s -check-prefix=UIN3_OVRLD
// UIN3_OVRLD: call i32 @dx.op.tertiary.i32(i32 49

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=uint2 -DELEM_TY2=uint2 -DRET_TY=uint %s  | FileCheck %s -check-prefix=UIN2_OVRLD
// UIN2_OVRLD: call i32 @dx.op.tertiary.i32(i32 49

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=uint -DELEM_TY2=uint -DRET_TY=uint %s  | FileCheck %s -check-prefix=UIN_OVRLD
// UIN_OVRLD: mul i32

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=uint4 -DELEM_TY2=uint4 -DRET_TY=uint %s  | FileCheck %s -check-prefix=UIN4_OVRLD_OD
// UIN4_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 49

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=uint3 -DELEM_TY2=uint3 -DRET_TY=uint %s  | FileCheck %s -check-prefix=UIN3_OVRLD_OD
// UIN3_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 49

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=uint2 -DELEM_TY2=uint2 -DRET_TY=uint %s  | FileCheck %s -check-prefix=UIN2_OVRLD_OD
// UIN2_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 49

// matrix
// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=uint2x4 -DELEM_TY2=uint4x3 -DRET_TY=uint2x3 %s  | FileCheck %s -check-prefix=UINMAT1_OVRLD
// UINMAT1_OVRLD: call i32 @dx.op.tertiary.i32(i32 49
// UINMAT1_OVRLD: call i32 @dx.op.tertiary.i32(i32 49
// UINMAT1_OVRLD: call i32 @dx.op.tertiary.i32(i32 49
// UINMAT1_OVRLD: mul i32

// UINMAT1_OVRLD: call i32 @dx.op.tertiary.i32(i32 49
// UINMAT1_OVRLD: call i32 @dx.op.tertiary.i32(i32 49
// UINMAT1_OVRLD: call i32 @dx.op.tertiary.i32(i32 49
// UINMAT1_OVRLD: mul i32

// UINMAT1_OVRLD: call i32 @dx.op.tertiary.i32(i32 49
// UINMAT1_OVRLD: call i32 @dx.op.tertiary.i32(i32 49
// UINMAT1_OVRLD: call i32 @dx.op.tertiary.i32(i32 49
// UINMAT1_OVRLD: mul i32

// UINMAT1_OVRLD: call i32 @dx.op.tertiary.i32(i32 49
// UINMAT1_OVRLD: call i32 @dx.op.tertiary.i32(i32 49
// UINMAT1_OVRLD: call i32 @dx.op.tertiary.i32(i32 49
// UINMAT1_OVRLD: mul i32

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=uint1x4 -DELEM_TY2=uint4x1 -DRET_TY=uint1x1 %s  | FileCheck %s -check-prefix=UINMAT2_OVRLD
// UINMAT2_OVRLD: mul i32
// UINMAT2_OVRLD: call i32 @dx.op.tertiary.i32(i32 49
// UINMAT2_OVRLD: call i32 @dx.op.tertiary.i32(i32 49
// UINMAT2_OVRLD: call i32 @dx.op.tertiary.i32(i32 49

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=uint2x4 -DELEM_TY2=uint4x3 -DRET_TY=uint2x3 %s  | FileCheck %s -check-prefix=UINMAT1_OVRLD_OD
// UINMAT1_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 49
// UINMAT1_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 49
// UINMAT1_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 49
// UINMAT1_OVRLD_OD: mul i32

// UINMAT1_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 49
// UINMAT1_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 49
// UINMAT1_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 49
// UINMAT1_OVRLD_OD: mul i32

// UINMAT1_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 49
// UINMAT1_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 49
// UINMAT1_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 49
// UINMAT1_OVRLD_OD: mul i32

// UINMAT1_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 49
// UINMAT1_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 49
// UINMAT1_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 49
// UINMAT1_OVRLD_OD: mul i32

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=uint1x4 -DELEM_TY2=uint4x1 -DRET_TY=uint1x1 %s  | FileCheck %s -check-prefix=UINMAT2_OVRLD_OD
// UINMAT2_OVRLD_OD: mul i32
// UINMAT2_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 49
// UINMAT2_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 49
// UINMAT2_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 49

// mixed: scalar and vector
// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=uint -DELEM_TY2=uint4 -DRET_TY=uint4 %s  | FileCheck %s -check-prefix=UIN1_4_OVRLD
// UIN1_4_OVRLD: mul i32
// UIN1_4_OVRLD: mul i32
// UIN1_4_OVRLD: mul i32
// UIN1_4_OVRLD: mul i32

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=uint3 -DELEM_TY2=uint -DRET_TY=uint3 %s  | FileCheck %s -check-prefix=UIN3_1_OVRLD
// UIN3_1_OVRLD: mul i32
// UIN3_1_OVRLD: mul i32
// UIN3_1_OVRLD: mul i32

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=uint -DELEM_TY2=uint4 -DRET_TY=uint4 %s  | FileCheck %s -check-prefix=UIN1_4_OVRLD_OD
// UIN1_4_OVRLD_OD: mul i32
// UIN1_4_OVRLD_OD: mul i32
// UIN1_4_OVRLD_OD: mul i32
// UIN1_4_OVRLD_OD: mul i32

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=uint3 -DELEM_TY2=uint -DRET_TY=uint3 %s  | FileCheck %s -check-prefix=UIN3_1_OVRLD_OD
// UIN3_1_OVRLD_OD: mul i32
// UIN3_1_OVRLD_OD: mul i32
// UIN3_1_OVRLD_OD: mul i32

// mixed: scalar and matrix
// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=uint -DELEM_TY2=uint2x4 -DRET_TY=uint2x4 %s  | FileCheck %s -check-prefix=UIN1_MAT1_OVRLD
// UIN1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT1_OVRLD: mul i32
// UIN1_MAT1_OVRLD: mul i32
// UIN1_MAT1_OVRLD: mul i32
// UIN1_MAT1_OVRLD: mul i32
// UIN1_MAT1_OVRLD: mul i32
// UIN1_MAT1_OVRLD: mul i32
// UIN1_MAT1_OVRLD: mul i32
// UIN1_MAT1_OVRLD: mul i32

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=uint4x3 -DELEM_TY2=uint -DRET_TY=uint4x3 %s  | FileCheck %s -check-prefix=UIN1_MAT2_OVRLD
// UIN1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT2_OVRLD: mul i32
// UIN1_MAT2_OVRLD: mul i32
// UIN1_MAT2_OVRLD: mul i32
// UIN1_MAT2_OVRLD: mul i32
// UIN1_MAT2_OVRLD: mul i32
// UIN1_MAT2_OVRLD: mul i32
// UIN1_MAT2_OVRLD: mul i32
// UIN1_MAT2_OVRLD: mul i32
// UIN1_MAT2_OVRLD: mul i32
// UIN1_MAT2_OVRLD: mul i32
// UIN1_MAT2_OVRLD: mul i32
// UIN1_MAT2_OVRLD: mul i32

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=uint -DELEM_TY2=uint2x4 -DRET_TY=uint2x4 %s  | FileCheck %s -check-prefix=UIN1_MAT1_OVRLD_OD
// UIN1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT1_OVRLD_OD: mul i32
// UIN1_MAT1_OVRLD_OD: mul i32
// UIN1_MAT1_OVRLD_OD: mul i32
// UIN1_MAT1_OVRLD_OD: mul i32
// UIN1_MAT1_OVRLD_OD: mul i32
// UIN1_MAT1_OVRLD_OD: mul i32
// UIN1_MAT1_OVRLD_OD: mul i32
// UIN1_MAT1_OVRLD_OD: mul i32

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=uint4x3 -DELEM_TY2=uint -DRET_TY=uint4x3 %s  | FileCheck %s -check-prefix=UIN1_MAT2_OVRLD_OD
// UIN1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.i32
// UIN1_MAT2_OVRLD_OD: mul i32
// UIN1_MAT2_OVRLD_OD: mul i32
// UIN1_MAT2_OVRLD_OD: mul i32
// UIN1_MAT2_OVRLD_OD: mul i32
// UIN1_MAT2_OVRLD_OD: mul i32
// UIN1_MAT2_OVRLD_OD: mul i32
// UIN1_MAT2_OVRLD_OD: mul i32
// UIN1_MAT2_OVRLD_OD: mul i32
// UIN1_MAT2_OVRLD_OD: mul i32
// UIN1_MAT2_OVRLD_OD: mul i32
// UIN1_MAT2_OVRLD_OD: mul i32
// UIN1_MAT2_OVRLD_OD: mul i32

// mixed: vector and matrix
// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=uint2 -DELEM_TY2=uint2x4 -DRET_TY=uint4 %s  | FileCheck %s -check-prefix=UIN2_MAT1_OVRLD
// UIN2_MAT1_OVRLD: mul i32
// UIN2_MAT1_OVRLD: call i32 @dx.op.tertiary.i32(i32 49
// UIN2_MAT1_OVRLD: mul i32
// UIN2_MAT1_OVRLD: call i32 @dx.op.tertiary.i32(i32 49
// UIN2_MAT1_OVRLD: mul i32
// UIN2_MAT1_OVRLD: call i32 @dx.op.tertiary.i32(i32 49
// UIN2_MAT1_OVRLD: mul i32

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=uint4x3 -DELEM_TY2=uint3 -DRET_TY=uint4 %s  | FileCheck %s -check-prefix=UIN3_MAT2_OVRLD
// UIN3_MAT2_OVRLD: mul i32
// UIN3_MAT2_OVRLD: call i32 @dx.op.tertiary.i32(i32 49
// UIN3_MAT2_OVRLD: call i32 @dx.op.tertiary.i32(i32 49
// UIN3_MAT2_OVRLD: mul i32
// UIN3_MAT2_OVRLD: call i32 @dx.op.tertiary.i32(i32 49
// UIN3_MAT2_OVRLD: call i32 @dx.op.tertiary.i32(i32 49
// UIN3_MAT2_OVRLD: mul i32
// UIN3_MAT2_OVRLD: call i32 @dx.op.tertiary.i32(i32 49
// UIN3_MAT2_OVRLD: call i32 @dx.op.tertiary.i32(i32 49
// UIN3_MAT2_OVRLD: mul i32
// UIN3_MAT2_OVRLD: call i32 @dx.op.tertiary.i32(i32 49
// UIN3_MAT2_OVRLD: call i32 @dx.op.tertiary.i32(i32 49

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=uint2 -DELEM_TY2=uint2x4 -DRET_TY=uint4 %s  | FileCheck %s -check-prefix=UIN2_MAT1_OVRLD_OD
// UIN2_MAT1_OVRLD_OD: mul i32
// UIN2_MAT1_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 49
// UIN2_MAT1_OVRLD_OD: mul i32
// UIN2_MAT1_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 49
// UIN2_MAT1_OVRLD_OD: mul i32
// UIN2_MAT1_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 49
// UIN2_MAT1_OVRLD_OD: mul i32

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=uint4x3 -DELEM_TY2=uint3 -DRET_TY=uint4 %s  | FileCheck %s -check-prefix=UIN3_MAT2_OVRLD_OD
// UIN3_MAT2_OVRLD_OD: mul i32
// UIN3_MAT2_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 49
// UIN3_MAT2_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 49
// UIN3_MAT2_OVRLD_OD: mul i32
// UIN3_MAT2_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 49
// UIN3_MAT2_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 49
// UIN3_MAT2_OVRLD_OD: mul i32
// UIN3_MAT2_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 49
// UIN3_MAT2_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 49
// UIN3_MAT2_OVRLD_OD: mul i32
// UIN3_MAT2_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 49
// UIN3_MAT2_OVRLD_OD: call i32 @dx.op.tertiary.i32(i32 49


// *****************************
// min16float overloads
// *****************************

// vectors and scalars
// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=min16float4 -DELEM_TY2=min16float4 -DRET_TY=min16float %s  | FileCheck %s -check-prefix=MNFL4_OVRLD
// MNFL4_OVRLD: call half @dx.op.dot4.f16(i32 56

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=min16float3 -DELEM_TY2=min16float3 -DRET_TY=min16float %s  | FileCheck %s -check-prefix=MNFL3_OVRLD
// MNFL3_OVRLD: call half @dx.op.dot3.f16

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=min16float2 -DELEM_TY2=min16float2 -DRET_TY=min16float %s  | FileCheck %s -check-prefix=MNFL2_OVRLD
// MNFL2_OVRLD: call half @dx.op.dot2.f16

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=min16float -DELEM_TY2=min16float -DRET_TY=min16float %s  | FileCheck %s -check-prefix=MNFL_OVRLD
// MNFL_OVRLD: fmul fast half

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=min16float4 -DELEM_TY2=min16float4 -DRET_TY=min16float %s  | FileCheck %s -check-prefix=MNFL4_OVRLD_OD
// MNFL4_OVRLD_OD: call half @dx.op.dot4.f16(i32 56

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=min16float3 -DELEM_TY2=min16float3 -DRET_TY=min16float %s  | FileCheck %s -check-prefix=MNFL3_OVRLD_OD
// MNFL3_OVRLD_OD: call half @dx.op.dot3.f16

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=min16float2 -DELEM_TY2=min16float2 -DRET_TY=min16float %s  | FileCheck %s -check-prefix=MNFL2_OVRLD_OD
// MNFL2_OVRLD_OD: call half @dx.op.dot2.f16

// matrix
// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=min16float2x4 -DELEM_TY2=min16float4x3 -DRET_TY=min16float2x3 %s  | FileCheck %s -check-prefix=MNFLMAT1_OVRLD
// MNFLMAT1_OVRLD: call half @dx.op.tertiary.f16(i32 46
// MNFLMAT1_OVRLD: call half @dx.op.tertiary.f16(i32 46
// MNFLMAT1_OVRLD: call half @dx.op.tertiary.f16(i32 46
// MNFLMAT1_OVRLD: fmul fast half

// MNFLMAT1_OVRLD: call half @dx.op.tertiary.f16(i32 46
// MNFLMAT1_OVRLD: call half @dx.op.tertiary.f16(i32 46
// MNFLMAT1_OVRLD: call half @dx.op.tertiary.f16(i32 46
// MNFLMAT1_OVRLD: fmul fast half

// MNFLMAT1_OVRLD: call half @dx.op.tertiary.f16(i32 46
// MNFLMAT1_OVRLD: call half @dx.op.tertiary.f16(i32 46
// MNFLMAT1_OVRLD: call half @dx.op.tertiary.f16(i32 46
// MNFLMAT1_OVRLD: fmul fast half

// MNFLMAT1_OVRLD: call half @dx.op.tertiary.f16(i32 46
// MNFLMAT1_OVRLD: call half @dx.op.tertiary.f16(i32 46
// MNFLMAT1_OVRLD: call half @dx.op.tertiary.f16(i32 46
// MNFLMAT1_OVRLD: fmul fast half

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=min16float1x4 -DELEM_TY2=min16float4x1 -DRET_TY=min16float1x1 %s  | FileCheck %s -check-prefix=MNFLMAT2_OVRLD
// MNFLMAT2_OVRLD: fmul fast half
// MNFLMAT2_OVRLD: call half @dx.op.tertiary.f16(i32 46
// MNFLMAT2_OVRLD: call half @dx.op.tertiary.f16(i32 46
// MNFLMAT2_OVRLD: call half @dx.op.tertiary.f16(i32 46

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=min16float2x4 -DELEM_TY2=min16float4x3 -DRET_TY=min16float2x3 %s  | FileCheck %s -check-prefix=MNFLMAT1_OVRLD_OD
// MNFLMAT1_OVRLD_OD: call half @dx.op.tertiary.f16(i32 46
// MNFLMAT1_OVRLD_OD: call half @dx.op.tertiary.f16(i32 46
// MNFLMAT1_OVRLD_OD: call half @dx.op.tertiary.f16(i32 46
// MNFLMAT1_OVRLD_OD: fmul fast half

// MNFLMAT1_OVRLD_OD: call half @dx.op.tertiary.f16(i32 46
// MNFLMAT1_OVRLD_OD: call half @dx.op.tertiary.f16(i32 46
// MNFLMAT1_OVRLD_OD: call half @dx.op.tertiary.f16(i32 46
// MNFLMAT1_OVRLD_OD: fmul fast half

// MNFLMAT1_OVRLD_OD: call half @dx.op.tertiary.f16(i32 46
// MNFLMAT1_OVRLD_OD: call half @dx.op.tertiary.f16(i32 46
// MNFLMAT1_OVRLD_OD: call half @dx.op.tertiary.f16(i32 46
// MNFLMAT1_OVRLD_OD: fmul fast half

// MNFLMAT1_OVRLD_OD: call half @dx.op.tertiary.f16(i32 46
// MNFLMAT1_OVRLD_OD: call half @dx.op.tertiary.f16(i32 46
// MNFLMAT1_OVRLD_OD: call half @dx.op.tertiary.f16(i32 46
// MNFLMAT1_OVRLD_OD: fmul fast half

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=min16float1x4 -DELEM_TY2=min16float4x1 -DRET_TY=min16float1x1 %s  | FileCheck %s -check-prefix=MNFLMAT2_OVRLD_OD
// MNFLMAT2_OVRLD_OD: fmul fast half
// MNFLMAT2_OVRLD_OD: call half @dx.op.tertiary.f16(i32 46
// MNFLMAT2_OVRLD_OD: call half @dx.op.tertiary.f16(i32 46
// MNFLMAT2_OVRLD_OD: call half @dx.op.tertiary.f16(i32 46

// mixed: scalar and vector
// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=min16float -DELEM_TY2=min16float4 -DRET_TY=min16float4 %s  | FileCheck %s -check-prefix=MNFL1_4_OVRLD
// MNFL1_4_OVRLD: fmul fast half
// MNFL1_4_OVRLD: fmul fast half
// MNFL1_4_OVRLD: fmul fast half
// MNFL1_4_OVRLD: fmul fast half

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=min16float3 -DELEM_TY2=min16float -DRET_TY=min16float3 %s  | FileCheck %s -check-prefix=MNFL3_1_OVRLD
// MNFL3_1_OVRLD: fmul fast half
// MNFL3_1_OVRLD: fmul fast half
// MNFL3_1_OVRLD: fmul fast half

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=min16float -DELEM_TY2=min16float4 -DRET_TY=min16float4 %s  | FileCheck %s -check-prefix=MNFL1_4_OVRLD_OD
// MNFL1_4_OVRLD_OD: fmul fast half
// MNFL1_4_OVRLD_OD: fmul fast half
// MNFL1_4_OVRLD_OD: fmul fast half
// MNFL1_4_OVRLD_OD: fmul fast half

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=min16float3 -DELEM_TY2=min16float -DRET_TY=min16float3 %s  | FileCheck %s -check-prefix=MNFL3_1_OVRLD_OD
// MNFL3_1_OVRLD_OD: fmul fast half
// MNFL3_1_OVRLD_OD: fmul fast half
// MNFL3_1_OVRLD_OD: fmul fast half

// mixed: scalar and matrix
// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=min16float -DELEM_TY2=min16float2x4 -DRET_TY=min16float2x4 %s  | FileCheck %s -check-prefix=MNFL1_MAT1_OVRLD
// MNFL1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT1_OVRLD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT1_OVRLD: fmul fast half
// MNFL1_MAT1_OVRLD: fmul fast half
// MNFL1_MAT1_OVRLD: fmul fast half
// MNFL1_MAT1_OVRLD: fmul fast half
// MNFL1_MAT1_OVRLD: fmul fast half
// MNFL1_MAT1_OVRLD: fmul fast half
// MNFL1_MAT1_OVRLD: fmul fast half
// MNFL1_MAT1_OVRLD: fmul fast half

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=min16float4x3 -DELEM_TY2=min16float -DRET_TY=min16float4x3 %s  | FileCheck %s -check-prefix=MNFL1_MAT2_OVRLD
// MNFL1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT2_OVRLD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT2_OVRLD: fmul fast half
// MNFL1_MAT2_OVRLD: fmul fast half
// MNFL1_MAT2_OVRLD: fmul fast half
// MNFL1_MAT2_OVRLD: fmul fast half
// MNFL1_MAT2_OVRLD: fmul fast half
// MNFL1_MAT2_OVRLD: fmul fast half
// MNFL1_MAT2_OVRLD: fmul fast half
// MNFL1_MAT2_OVRLD: fmul fast half
// MNFL1_MAT2_OVRLD: fmul fast half
// MNFL1_MAT2_OVRLD: fmul fast half
// MNFL1_MAT2_OVRLD: fmul fast half
// MNFL1_MAT2_OVRLD: fmul fast half

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=min16float -DELEM_TY2=min16float2x4 -DRET_TY=min16float2x4 %s  | FileCheck %s -check-prefix=MNFL1_MAT1_OVRLD_OD
// MNFL1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT1_OVRLD_OD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT1_OVRLD_OD: fmul fast half
// MNFL1_MAT1_OVRLD_OD: fmul fast half
// MNFL1_MAT1_OVRLD_OD: fmul fast half
// MNFL1_MAT1_OVRLD_OD: fmul fast half
// MNFL1_MAT1_OVRLD_OD: fmul fast half
// MNFL1_MAT1_OVRLD_OD: fmul fast half
// MNFL1_MAT1_OVRLD_OD: fmul fast half
// MNFL1_MAT1_OVRLD_OD: fmul fast half

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=min16float4x3 -DELEM_TY2=min16float -DRET_TY=min16float4x3 %s  | FileCheck %s -check-prefix=MNFL1_MAT2_OVRLD_OD
// MNFL1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT2_OVRLD_OD: extractvalue %dx.types.CBufRet.f16
// MNFL1_MAT2_OVRLD_OD: fmul fast half
// MNFL1_MAT2_OVRLD_OD: fmul fast half
// MNFL1_MAT2_OVRLD_OD: fmul fast half
// MNFL1_MAT2_OVRLD_OD: fmul fast half
// MNFL1_MAT2_OVRLD_OD: fmul fast half
// MNFL1_MAT2_OVRLD_OD: fmul fast half
// MNFL1_MAT2_OVRLD_OD: fmul fast half
// MNFL1_MAT2_OVRLD_OD: fmul fast half
// MNFL1_MAT2_OVRLD_OD: fmul fast half
// MNFL1_MAT2_OVRLD_OD: fmul fast half
// MNFL1_MAT2_OVRLD_OD: fmul fast half
// MNFL1_MAT2_OVRLD_OD: fmul fast half

// mixed: vector and matrix
// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=min16float2 -DELEM_TY2=min16float2x4 -DRET_TY=min16float4 %s  | FileCheck %s -check-prefix=MNFL2_MAT1_OVRLD
// MNFL2_MAT1_OVRLD: fmul fast half
// MNFL2_MAT1_OVRLD: call half @dx.op.tertiary.f16(i32 46
// MNFL2_MAT1_OVRLD: fmul fast half
// MNFL2_MAT1_OVRLD: call half @dx.op.tertiary.f16(i32 46
// MNFL2_MAT1_OVRLD: fmul fast half
// MNFL2_MAT1_OVRLD: call half @dx.op.tertiary.f16(i32 46
// MNFL2_MAT1_OVRLD: fmul fast half

// RUN: %dxc -T vs_6_0 -E main -DELEM_TY1=min16float4x3 -DELEM_TY2=min16float3 -DRET_TY=min16float4 %s  | FileCheck %s -check-prefix=MNFL3_MAT2_OVRLD
// MNFL3_MAT2_OVRLD: fmul fast half
// MNFL3_MAT2_OVRLD: call half @dx.op.tertiary.f16(i32 46
// MNFL3_MAT2_OVRLD: call half @dx.op.tertiary.f16(i32 46
// MNFL3_MAT2_OVRLD: fmul fast half
// MNFL3_MAT2_OVRLD: call half @dx.op.tertiary.f16(i32 46
// MNFL3_MAT2_OVRLD: call half @dx.op.tertiary.f16(i32 46
// MNFL3_MAT2_OVRLD: fmul fast half
// MNFL3_MAT2_OVRLD: call half @dx.op.tertiary.f16(i32 46
// MNFL3_MAT2_OVRLD: call half @dx.op.tertiary.f16(i32 46
// MNFL3_MAT2_OVRLD: fmul fast half
// MNFL3_MAT2_OVRLD: call half @dx.op.tertiary.f16(i32 46
// MNFL3_MAT2_OVRLD: call half @dx.op.tertiary.f16(i32 46

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=min16float2 -DELEM_TY2=min16float2x4 -DRET_TY=min16float4 %s  | FileCheck %s -check-prefix=MNFL2_MAT1_OVRLD_OD
// MNFL2_MAT1_OVRLD_OD: fmul fast half
// MNFL2_MAT1_OVRLD_OD: call half @dx.op.tertiary.f16(i32 46
// MNFL2_MAT1_OVRLD_OD: fmul fast half
// MNFL2_MAT1_OVRLD_OD: call half @dx.op.tertiary.f16(i32 46
// MNFL2_MAT1_OVRLD_OD: fmul fast half
// MNFL2_MAT1_OVRLD_OD: call half @dx.op.tertiary.f16(i32 46
// MNFL2_MAT1_OVRLD_OD: fmul fast half

// RUN: %dxc -T vs_6_0 -E main -Od -DELEM_TY1=min16float4x3 -DELEM_TY2=min16float3 -DRET_TY=min16float4 %s  | FileCheck %s -check-prefix=MNFL3_MAT2_OVRLD_OD
// MNFL3_MAT2_OVRLD_OD: fmul fast half
// MNFL3_MAT2_OVRLD_OD: call half @dx.op.tertiary.f16(i32 46
// MNFL3_MAT2_OVRLD_OD: call half @dx.op.tertiary.f16(i32 46
// MNFL3_MAT2_OVRLD_OD: fmul fast half
// MNFL3_MAT2_OVRLD_OD: call half @dx.op.tertiary.f16(i32 46
// MNFL3_MAT2_OVRLD_OD: call half @dx.op.tertiary.f16(i32 46
// MNFL3_MAT2_OVRLD_OD: fmul fast half
// MNFL3_MAT2_OVRLD_OD: call half @dx.op.tertiary.f16(i32 46
// MNFL3_MAT2_OVRLD_OD: call half @dx.op.tertiary.f16(i32 46
// MNFL3_MAT2_OVRLD_OD: fmul fast half
// MNFL3_MAT2_OVRLD_OD: call half @dx.op.tertiary.f16(i32 46
// MNFL3_MAT2_OVRLD_OD: call half @dx.op.tertiary.f16(i32 46

cbuffer CB {
  ELEM_TY1 e1;
  ELEM_TY2 e2;
};

RET_TY main(): OUT
{
	return mul(e1, e2);
}