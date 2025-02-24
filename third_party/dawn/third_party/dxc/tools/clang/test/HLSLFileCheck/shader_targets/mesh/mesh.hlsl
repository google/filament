// RUN: %dxc -E main -T ms_6_5 %s | FileCheck %s
// RUN: %dxc -E main -T ms_6_5 %s | %D3DReflect %s | FileCheck -check-prefix=REFL %s

// CHECK: Mesh Shader
// CHECK: MeshOutputTopology=triangle
// CHECK: NumThreads=(32,1,1)
// CHECK: dx.op.getMeshPayload.struct.MeshPayload(i32 170)
// CHECK: dx.op.setMeshOutputCounts(i32 168, i32 32, i32 16)
// CHECK: dx.op.emitIndices(i32 169,
// CHECK: dx.op.storePrimitiveOutput.f32(i32 172,
// CHECK: dx.op.storeVertexOutput.f32(i32 171,
// CHECK: !{i32 5, !"SV_CullPrimitive", i8 1, i8 30, !{{[0-9]+}}, i8 1, i32 1, i8 1, i32 -1, i8 -1, !{{[0-9]+}}}

#define MAX_VERT 32
#define MAX_PRIM 16
#define NUM_THREADS 32
struct MeshPerVertex {
    float4 position : SV_Position;
    float color[4] : COLOR;
};

struct MeshPerPrimitive {
    float normal : NORMAL;
    float malnor : MALNOR;
    float alnorm : ALNORM;
    float ormaln : ORMALN;
    int layer[6] : LAYER;
    bool cullPrimitive : SV_CullPrimitive;
};

struct MeshPayload {
    float normal;
    float malnor;
    float alnorm;
    float ormaln;
    int layer[6];
};

groupshared float gsMem[MAX_PRIM];

[numthreads(NUM_THREADS, 1, 1)]
[outputtopology("triangle")]
void main(
            out indices uint3 primIndices[MAX_PRIM],
            out vertices MeshPerVertex verts[MAX_VERT],
            out primitives MeshPerPrimitive prims[MAX_PRIM],
            in payload MeshPayload mpl,
            in uint tig : SV_GroupIndex,
            in uint vid : SV_ViewID
         )
{
    SetMeshOutputCounts(MAX_VERT, MAX_PRIM);
    MeshPerVertex ov;
    if (vid % 2) {
        ov.position = float4(4.0,5.0,6.0,7.0);
        ov.color[0] = 4.0;
        ov.color[1] = 5.0;
        ov.color[2] = 6.0;
        ov.color[3] = 7.0;
    } else {
        ov.position = float4(14.0,15.0,16.0,17.0);
        ov.color[0] = 14.0;
        ov.color[1] = 15.0;
        ov.color[2] = 16.0;
        ov.color[3] = 17.0;
    }
    if (tig % 3) {
      primIndices[tig / 3] = uint3(tig, tig + 1, tig + 2);
      MeshPerPrimitive op;
      op.normal = mpl.normal;
      op.malnor = gsMem[tig / 3 + 1];
      op.alnorm = mpl.alnorm;
      op.ormaln = mpl.ormaln;
      op.layer[0] = mpl.layer[0];
      op.layer[1] = mpl.layer[1];
      op.layer[2] = mpl.layer[2];
      op.layer[3] = mpl.layer[3];
      op.layer[4] = mpl.layer[4];
      op.layer[5] = mpl.layer[5];
      op.cullPrimitive = false;
      gsMem[tig / 3] = op.normal;
      prims[tig / 3] = op;
    }
    verts[tig] = ov;
}

// REFL: InputParameters: 0
// REFL: OutputParameters: 5
// REFL: PatchConstantParameters: 11
// REFL: TempArrayCount: 64
// REFL: DynamicFlowControlCount: 1
// REFL: ArrayInstructionCount: 2
// REFL: TextureNormalInstructions: 0
// REFL: TextureLoadInstructions: 0
// REFL: TextureCompInstructions: 0
// REFL: TextureBiasInstructions: 0
// REFL: TextureGradientInstructions: 0
// REFL: FloatInstructionCount: 0
// REFL: IntInstructionCount: 5
// REFL: UintInstructionCount: 3
// REFL: CutInstructionCount: 0
// REFL: EmitInstructionCount: 0
// REFL: cBarrierInstructions: 0
// REFL: cInterlockedInstructions: 0
// REFL: cTextureStoreInstructions: 0

// REFL:  OutputParameter Elements: 5
// REFL-NEXT:    D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: SV_POSITION SemanticIndex: 0
// REFL-NEXT:      Register: 0
// REFL-NEXT:      SystemValueType: D3D_NAME_POSITION
// REFL-NEXT:      ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// REFL-NEXT:      Mask: xyzw
// REFL-NEXT:      ReadWriteMask: ----
// REFL-NEXT:      Stream: 0
// REFL-NEXT:      MinPrecision: D3D_MIN_PRECISION_DEFAULT
// REFL-NEXT:    D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: COLOR SemanticIndex: 0
// REFL-NEXT:      Register: 1
// REFL-NEXT:      SystemValueType: D3D_NAME_UNDEFINED
// REFL-NEXT:      ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// REFL-NEXT:      Mask: x---
// REFL-NEXT:      ReadWriteMask: -yzw
// REFL-NEXT:      Stream: 0
// REFL-NEXT:      MinPrecision: D3D_MIN_PRECISION_DEFAULT
// REFL-NEXT:    D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: COLOR SemanticIndex: 1
// REFL-NEXT:      Register: 2
// REFL-NEXT:      SystemValueType: D3D_NAME_UNDEFINED
// REFL-NEXT:      ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// REFL-NEXT:      Mask: x---
// REFL-NEXT:      ReadWriteMask: -yzw
// REFL-NEXT:      Stream: 0
// REFL-NEXT:      MinPrecision: D3D_MIN_PRECISION_DEFAULT
// REFL-NEXT:    D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: COLOR SemanticIndex: 2
// REFL-NEXT:      Register: 3
// REFL-NEXT:      SystemValueType: D3D_NAME_UNDEFINED
// REFL-NEXT:      ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// REFL-NEXT:      Mask: x---
// REFL-NEXT:      ReadWriteMask: -yzw
// REFL-NEXT:      Stream: 0
// REFL-NEXT:      MinPrecision: D3D_MIN_PRECISION_DEFAULT
// REFL-NEXT:    D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: COLOR SemanticIndex: 3
// REFL-NEXT:      Register: 4
// REFL-NEXT:      SystemValueType: D3D_NAME_UNDEFINED
// REFL-NEXT:      ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// REFL-NEXT:      Mask: x---
// REFL-NEXT:      ReadWriteMask: -yzw
// REFL-NEXT:      Stream: 0
// REFL-NEXT:      MinPrecision: D3D_MIN_PRECISION_DEFAULT
// REFL-NEXT:  PatchConstantParameter Elements: 11
// REFL-NEXT:    D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: NORMAL SemanticIndex: 0
// REFL-NEXT:      Register: 0
// REFL-NEXT:      SystemValueType: D3D_NAME_UNDEFINED
// REFL-NEXT:      ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// REFL-NEXT:      Mask: x---
// REFL-NEXT:      ReadWriteMask: -yzw
// REFL-NEXT:      Stream: 0
// REFL-NEXT:      MinPrecision: D3D_MIN_PRECISION_DEFAULT
// REFL-NEXT:    D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: MALNOR SemanticIndex: 0
// REFL-NEXT:      Register: 0
// REFL-NEXT:      SystemValueType: D3D_NAME_UNDEFINED
// REFL-NEXT:      ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// REFL-NEXT:      Mask: -y--
// REFL-NEXT:      ReadWriteMask: x-zw
// REFL-NEXT:      Stream: 0
// REFL-NEXT:      MinPrecision: D3D_MIN_PRECISION_DEFAULT
// REFL-NEXT:    D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: ALNORM SemanticIndex: 0
// REFL-NEXT:      Register: 0
// REFL-NEXT:      SystemValueType: D3D_NAME_UNDEFINED
// REFL-NEXT:      ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// REFL-NEXT:      Mask: --z-
// REFL-NEXT:      ReadWriteMask: xy-w
// REFL-NEXT:      Stream: 0
// REFL-NEXT:      MinPrecision: D3D_MIN_PRECISION_DEFAULT
// REFL-NEXT:    D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: ORMALN SemanticIndex: 0
// REFL-NEXT:      Register: 0
// REFL-NEXT:      SystemValueType: D3D_NAME_UNDEFINED
// REFL-NEXT:      ComponentType: D3D_REGISTER_COMPONENT_FLOAT32
// REFL-NEXT:      Mask: ---w
// REFL-NEXT:      ReadWriteMask: xyz-
// REFL-NEXT:      Stream: 0
// REFL-NEXT:      MinPrecision: D3D_MIN_PRECISION_DEFAULT
// REFL-NEXT:    D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: LAYER SemanticIndex: 0
// REFL-NEXT:      Register: 1
// REFL-NEXT:      SystemValueType: D3D_NAME_UNDEFINED
// REFL-NEXT:      ComponentType: D3D_REGISTER_COMPONENT_SINT32
// REFL-NEXT:      Mask: x---
// REFL-NEXT:      ReadWriteMask: -yzw
// REFL-NEXT:      Stream: 0
// REFL-NEXT:      MinPrecision: D3D_MIN_PRECISION_DEFAULT
// REFL-NEXT:    D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: LAYER SemanticIndex: 1
// REFL-NEXT:      Register: 2
// REFL-NEXT:      SystemValueType: D3D_NAME_UNDEFINED
// REFL-NEXT:      ComponentType: D3D_REGISTER_COMPONENT_SINT32
// REFL-NEXT:      Mask: x---
// REFL-NEXT:      ReadWriteMask: -yzw
// REFL-NEXT:      Stream: 0
// REFL-NEXT:      MinPrecision: D3D_MIN_PRECISION_DEFAULT
// REFL-NEXT:    D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: LAYER SemanticIndex: 2
// REFL-NEXT:      Register: 3
// REFL-NEXT:      SystemValueType: D3D_NAME_UNDEFINED
// REFL-NEXT:      ComponentType: D3D_REGISTER_COMPONENT_SINT32
// REFL-NEXT:      Mask: x---
// REFL-NEXT:      ReadWriteMask: -yzw
// REFL-NEXT:      Stream: 0
// REFL-NEXT:      MinPrecision: D3D_MIN_PRECISION_DEFAULT
// REFL-NEXT:    D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: LAYER SemanticIndex: 3
// REFL-NEXT:      Register: 4
// REFL-NEXT:      SystemValueType: D3D_NAME_UNDEFINED
// REFL-NEXT:      ComponentType: D3D_REGISTER_COMPONENT_SINT32
// REFL-NEXT:      Mask: x---
// REFL-NEXT:      ReadWriteMask: -yzw
// REFL-NEXT:      Stream: 0
// REFL-NEXT:      MinPrecision: D3D_MIN_PRECISION_DEFAULT
// REFL-NEXT:    D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: LAYER SemanticIndex: 4
// REFL-NEXT:      Register: 5
// REFL-NEXT:      SystemValueType: D3D_NAME_UNDEFINED
// REFL-NEXT:      ComponentType: D3D_REGISTER_COMPONENT_SINT32
// REFL-NEXT:      Mask: x---
// REFL-NEXT:      ReadWriteMask: -yzw
// REFL-NEXT:      Stream: 0
// REFL-NEXT:      MinPrecision: D3D_MIN_PRECISION_DEFAULT
// REFL-NEXT:    D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: LAYER SemanticIndex: 5
// REFL-NEXT:      Register: 6
// REFL-NEXT:      SystemValueType: D3D_NAME_UNDEFINED
// REFL-NEXT:      ComponentType: D3D_REGISTER_COMPONENT_SINT32
// REFL-NEXT:      Mask: x---
// REFL-NEXT:      ReadWriteMask: -yzw
// REFL-NEXT:      Stream: 0
// REFL-NEXT:      MinPrecision: D3D_MIN_PRECISION_DEFAULT
// REFL-NEXT:    D3D12_SIGNATURE_PARAMETER_DESC: SemanticName: SV_CULLPRIMITIVE SemanticIndex: 0
// REFL-NEXT:      Register: 4294967295
// REFL-NEXT:      SystemValueType: D3D_NAME_CULLPRIMITIVE
// REFL-NEXT:      ComponentType: D3D_REGISTER_COMPONENT_UINT32
// REFL-NEXT:      Mask: x---
// REFL-NEXT:      ReadWriteMask: -yzw
// REFL-NEXT:      Stream: 0
// REFL-NEXT:      MinPrecision: D3D_MIN_PRECISION_DEFAULT
