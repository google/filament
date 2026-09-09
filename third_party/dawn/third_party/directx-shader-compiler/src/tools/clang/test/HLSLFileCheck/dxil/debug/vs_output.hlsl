// RUN: %dxc /T vs_6_0 /E BezierVS /DDX12 /Od /Zi %s | FileCheck %s

// CHECK-DAG: call void @llvm.dbg.value(metadata float %{{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"output" !DIExpression(DW_OP_bit_piece, 0, 32)
// CHECK-DAG: call void @llvm.dbg.value(metadata float %{{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"output" !DIExpression(DW_OP_bit_piece, 32, 32)
// CHECK-DAG: call void @llvm.dbg.value(metadata float %{{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"output" !DIExpression(DW_OP_bit_piece, 64, 32)

//--------------------------------------------------------------------------------------
// SimpleBezier.hlsl
//
// Shader demonstrating DirectX 11 tesselation of a bezier surface
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#ifdef DX12
#define RS \
[RootSignature(" \
    RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), \
    DescriptorTable(CBV(b0, numDescriptors = 1), visibility=SHADER_VISIBILITY_ALL) \
")]
#else
#define RS
#endif

//--------------------------------------------------------------------------------------
// Constant Buffers
//--------------------------------------------------------------------------------------
cbuffer cbPerFrame : register( b0 )
{
    matrix g_mViewProjection;
    float3 g_cameraWorldPos;
    float  g_tessellationFactor;
};

//--------------------------------------------------------------------------------------
// Vertex shader section
//--------------------------------------------------------------------------------------
struct VS_CONTROL_POINT_INPUT
{
    float3 pos      : POSITION;
};

struct VS_CONTROL_POINT_OUTPUT
{
    float3 pos      : POSITION;
};

//--------------------------------------------------------------------------------------
// This simple vertex shader passes the control points straight through to the
// hull shader.  In a more complex scene, you might transform the control points
// or perform skinning at this step.

// The input to the vertex shader comes from the vertex buffer.

// The output from the vertex shader will go into the hull shader.
//--------------------------------------------------------------------------------------
RS
VS_CONTROL_POINT_OUTPUT BezierVS( VS_CONTROL_POINT_INPUT Input )
{
    VS_CONTROL_POINT_OUTPUT output;

    output.pos = Input.pos;

    return output;
}

