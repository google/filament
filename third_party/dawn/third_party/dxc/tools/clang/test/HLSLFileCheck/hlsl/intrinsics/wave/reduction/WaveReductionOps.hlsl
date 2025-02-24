// RUN: %dxc -E test_wavereadlaneat -T vs_6_0 /DTYPE=float /DRET_TYPE=float %s | FileCheck %s -check-prefix=RDLNAT_FLT
// RUN: %dxc -E test_wavereadlaneat -T vs_6_2 -enable-16bit-types /DTYPE=half /DRET_TYPE=half %s | FileCheck %s -check-prefix=RDLNAT_HALF
// RUN: %dxc -E test_wavereadlaneat -T vs_6_0 /DTYPE=min16float /DRET_TYPE=min16float %s | FileCheck %s -check-prefix=RDLNAT_MIN16FLT
// RUN: %dxc -E test_wavereadlaneat -T vs_6_0 /DTYPE=int /DRET_TYPE=int %s | FileCheck %s -check-prefix=RDLNAT_INT
// RUN: %dxc -E test_wavereadlaneat -T vs_6_0 /DTYPE=uint /DRET_TYPE=uint %s | FileCheck %s -check-prefix=RDLNAT_UINT
// RUN: %dxc -E test_wavereadlaneat -T vs_6_0 /DTYPE=double /DRET_TYPE=float %s | FileCheck %s -check-prefix=RDLNAT_DBL
// RUN: %dxc -E test_wavereadlaneat -T vs_6_0 /DTYPE=bool /DRET_TYPE=bool %s | FileCheck %s -check-prefix=RDLNAT_BOOL
// RUN: %dxc -E test_wavereadlaneat -T vs_6_0 /DTYPE=bool3 /DRET_TYPE=bool3 %s | FileCheck %s -check-prefix=RDLNAT_BOOL_VEC3
// RUN: %dxc -E test_wavereadlaneat -T vs_6_0 /DTYPE=int2x3 /DRET_TYPE=int %s | FileCheck %s -check-prefix=RDLNAT_MAT
// RUN: %dxc -E test_wavereadlanefirst -T vs_6_0 /DTYPE=float /DRET_TYPE=float %s | FileCheck %s -check-prefix=RDLNFRST_FLT
// RUN: %dxc -E test_wavereadlanefirst -T vs_6_2 -enable-16bit-types /DTYPE=half /DRET_TYPE=half %s | FileCheck %s -check-prefix=RDLNFRST_HALF
// RUN: %dxc -E test_wavereadlanefirst -T vs_6_0 /DTYPE=min16float /DRET_TYPE=min16float %s | FileCheck %s -check-prefix=RDLNFRST_MIN16FLT
// RUN: %dxc -E test_wavereadlanefirst -T vs_6_0 /DTYPE=int /DRET_TYPE=int %s | FileCheck %s -check-prefix=RDLNFRST_INT
// RUN: %dxc -E test_wavereadlanefirst -T vs_6_0 /DTYPE=uint /DRET_TYPE=uint %s | FileCheck %s -check-prefix=RDLNFRST_UINT
// RUN: %dxc -E test_wavereadlanefirst -T vs_6_0 /DTYPE=bool /DRET_TYPE=bool %s | FileCheck %s -check-prefix=RDLNFRST_BOOL
// RUN: %dxc -E test_wavereadlanefirst -T vs_6_0 /DTYPE=bool3 /DRET_TYPE=bool3 %s | FileCheck %s -check-prefix=RDLNFRST_BOOL_VEC3
// RUN: %dxc -E test_wavereadlanefirst -T vs_6_0 /DTYPE=int2x3 /DRET_TYPE=int %s | FileCheck %s -check-prefix=RDLNFRST_MAT

// This file should contain tests to cover all supported overloads of WaveIntrinsics used for reduction operations
// TODO: Currently only covers WaveReadFirstLane and WaveReadLaneAt. Add coverage for others.
// TODO: Add related coverage once these bugs are fixed: bug# 2501

cbuffer CB
{
  TYPE expr;
}

// RDLNAT_FLT: call float @dx.op.waveReadLaneAt.f32(i32 117, float
// RDLNAT_HALF: call half @dx.op.waveReadLaneAt.f16(i32 117, half
// RDLNAT_MIN16FLT: call half @dx.op.waveReadLaneAt.f16(i32 117, half
// RDLNAT_INT: call i32 @dx.op.waveReadLaneAt.i32(i32 117, i32
// RDLNAT_UINT: call i32 @dx.op.waveReadLaneAt.i32(i32 117, i32
// RDLNAT_DBL: call double @dx.op.waveReadLaneAt.f64(i32 117, double
// RDLNAT_BOOL: call i1 @dx.op.waveReadLaneAt.i1(i32 117, i1
// RDLNAT_BOOL_VEC3: call i1 @dx.op.waveReadLaneAt.i1(i32 117, i1
// RDLNAT_BOOL_VEC3: call i1 @dx.op.waveReadLaneAt.i1(i32 117, i1
// RDLNAT_BOOL_VEC3: call i1 @dx.op.waveReadLaneAt.i1(i32 117, i1
// RDLNAT_MAT: call i32 @dx.op.waveReadLaneAt.i32(i32 117, i32

RET_TYPE test_wavereadlaneat(uint id: IN0) : OUT
{
  return WaveReadLaneAt(expr, id);
}


// RDLNFRST_FLT: call float @dx.op.waveReadLaneFirst.f32(i32 118, float
// RDLNFRST_HALF: call half @dx.op.waveReadLaneFirst.f16(i32 118, half
// RDLNFRST_MIN16FLT: call half @dx.op.waveReadLaneFirst.f16(i32 118, half
// RDLNFRST_INT: call i32 @dx.op.waveReadLaneFirst.i32(i32 118, i32
// RDLNFRST_UINT: call i32 @dx.op.waveReadLaneFirst.i32(i32 118, i32
// RDLNFRST_BOOL: call i1 @dx.op.waveReadLaneFirst.i1(i32 118, i1
// RDLNFRST_BOOL_VEC3: call i1 @dx.op.waveReadLaneFirst.i1(i32 118, i1
// RDLNFRST_BOOL_VEC3: call i1 @dx.op.waveReadLaneFirst.i1(i32 118, i1
// RDLNFRST_BOOL_VEC3: call i1 @dx.op.waveReadLaneFirst.i1(i32 118, i1
// RDLNFRST_MAT: call i32 @dx.op.waveReadLaneFirst.i32(i32 118, i32
RET_TYPE test_wavereadlanefirst() : OUT
{
  return WaveReadLaneFirst(expr);
}