// RUN: %dxc /T vs_6_0 /E BezierVS /DDX12 /Od /Zi %s | FileCheck %s

// CHECK-DAG: call void @llvm.dbg.value(metadata float %{{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"output" !DIExpression(DW_OP_bit_piece, 0, 32)
// CHECK-DAG: call void @llvm.dbg.value(metadata float %{{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"output" !DIExpression(DW_OP_bit_piece, 32, 32)
// CHECK-DAG: call void @llvm.dbg.value(metadata float %{{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"output" !DIExpression(DW_OP_bit_piece, 64, 32)

//--------------------------------------------------------------------------------------
// vs_output2.hlsl
//
// variation on vs_output.hlsl that breaks up the basic blocks 
// so that the llvm.dbg.declare and the store are in different scopes
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

void SubProg(out VS_CONTROL_POINT_OUTPUT output, VS_CONTROL_POINT_INPUT input)
{
  output.pos = input.pos;
  if (output.pos.z > 3)
    output.pos.z = 1;
}

RS
VS_CONTROL_POINT_OUTPUT BezierVS( VS_CONTROL_POINT_INPUT input )
{
    VS_CONTROL_POINT_OUTPUT output;

    SubProg(output, input);
    return output;
}

