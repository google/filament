// REQUIRES: dxil-1-8
// RUN: %dxc -E main -T ms_6_8 %s -Fo %t
// RUN: %dxa %t -dumppsv | FileCheck %s

// CHECK:DxilPipelineStateValidation:
// CHECK-NEXT: PSVRuntimeInfo:
// CHECK-NEXT:  Mesh Shader
// CHECK-NEXT:  MeshOutputTopology=triangle
// CHECK-NEXT:  NumThreads=(32,1,1)
// CHECK-NEXT:  MinimumExpectedWaveLaneCount: 0
// CHECK-NEXT:  MaximumExpectedWaveLaneCount: 4294967295
// CHECK-NEXT:  UsesViewID: true
// CHECK-NEXT:  SigInputElements: 0
// CHECK-NEXT:  SigOutputElements: 2
// CHECK-NEXT:  SigPatchConstOrPrimElements: 6
// CHECK-NEXT:  SigInputVectors: 0
// CHECK-NEXT:  SigOutputVectors[0]: 5
// CHECK-NEXT:  SigOutputVectors[1]: 0
// CHECK-NEXT:  SigOutputVectors[2]: 0
// CHECK-NEXT:  SigOutputVectors[3]: 0
// CHECK-NEXT:  EntryFunctionName: main
// CHECK-NEXT: ResourceCount : 0
// CHECK-NEXT:  PSVSignatureElement:
// CHECK-NEXT:   SemanticName:
// CHECK-NEXT:   SemanticIndex: 0
// CHECK-NEXT:   IsAllocated: 1
// CHECK-NEXT:   StartRow: 0
// CHECK-NEXT:   StartCol: 0
// CHECK-NEXT:   Rows: 1
// CHECK-NEXT:   Cols: 4
// CHECK-NEXT:   SemanticKind: Position
// CHECK-NEXT:   InterpolationMode: 4
// CHECK-NEXT:   OutputStream: 0
// CHECK-NEXT:   ComponentType: 3
// CHECK-NEXT:   DynamicIndexMask: 0
// CHECK-NEXT: PSVSignatureElement:
// CHECK-NEXT:   SemanticName: COLOR
// CHECK-NEXT:   SemanticIndex: 0 1 2 3
// CHECK-NEXT:   IsAllocated: 1
// CHECK-NEXT:   StartRow: 1
// CHECK-NEXT:   StartCol: 0
// CHECK-NEXT:   Rows: 4
// CHECK-NEXT:   Cols: 1
// CHECK-NEXT:   SemanticKind: Arbitrary
// CHECK-NEXT:   InterpolationMode: 2
// CHECK-NEXT:   OutputStream: 0
// CHECK-NEXT:   ComponentType: 3
// CHECK-NEXT:   DynamicIndexMask: 0
// CHECK-NEXT: PSVSignatureElement:
// CHECK-NEXT:   SemanticName: NORMAL
// CHECK-NEXT:   SemanticIndex: 0
// CHECK-NEXT:   IsAllocated: 1
// CHECK-NEXT:   StartRow: 0
// CHECK-NEXT:   StartCol: 0
// CHECK-NEXT:   Rows: 1
// CHECK-NEXT:   Cols: 1
// CHECK-NEXT:   SemanticKind: Arbitrary
// CHECK-NEXT:   InterpolationMode: 1
// CHECK-NEXT:   OutputStream: 0
// CHECK-NEXT:   ComponentType: 3
// CHECK-NEXT:   DynamicIndexMask: 0
// CHECK-NEXT: PSVSignatureElement:
// CHECK-NEXT:   SemanticName: MALNOR
// CHECK-NEXT:   SemanticIndex: 0
// CHECK-NEXT:   IsAllocated: 1
// CHECK-NEXT:   StartRow: 0
// CHECK-NEXT:   StartCol: 1
// CHECK-NEXT:   Rows: 1
// CHECK-NEXT:   Cols: 1
// CHECK-NEXT:   SemanticKind: Arbitrary
// CHECK-NEXT:   InterpolationMode: 1
// CHECK-NEXT:   OutputStream: 0
// CHECK-NEXT:   ComponentType: 3
// CHECK-NEXT:   DynamicIndexMask: 0
// CHECK-NEXT: PSVSignatureElement:
// CHECK-NEXT:   SemanticName: ALNORM
// CHECK-NEXT:   SemanticIndex: 0
// CHECK-NEXT:   IsAllocated: 1
// CHECK-NEXT:   StartRow: 0
// CHECK-NEXT:   StartCol: 2
// CHECK-NEXT:   Rows: 1
// CHECK-NEXT:   Cols: 1
// CHECK-NEXT:   SemanticKind: Arbitrary
// CHECK-NEXT:   InterpolationMode: 1
// CHECK-NEXT:   OutputStream: 0
// CHECK-NEXT:   ComponentType: 3
// CHECK-NEXT:   DynamicIndexMask: 0
// CHECK-NEXT: PSVSignatureElement:
// CHECK-NEXT:   SemanticName: ORMALN
// CHECK-NEXT:   SemanticIndex: 0
// CHECK-NEXT:   IsAllocated: 1
// CHECK-NEXT:   StartRow: 0
// CHECK-NEXT:   StartCol: 3
// CHECK-NEXT:   Rows: 1
// CHECK-NEXT:   Cols: 1
// CHECK-NEXT:   SemanticKind: Arbitrary
// CHECK-NEXT:   InterpolationMode: 1
// CHECK-NEXT:   OutputStream: 0
// CHECK-NEXT:   ComponentType: 3
// CHECK-NEXT:   DynamicIndexMask: 0
// CHECK-NEXT: PSVSignatureElement:
// CHECK-NEXT:   SemanticName: LAYER
// CHECK-NEXT:   SemanticIndex: 0 1 2 3 4 5
// CHECK-NEXT:   IsAllocated: 1
// CHECK-NEXT:   StartRow: 1
// CHECK-NEXT:   StartCol: 0
// CHECK-NEXT:   Rows: 6
// CHECK-NEXT:   Cols: 1
// CHECK-NEXT:   SemanticKind: Arbitrary
// CHECK-NEXT:   InterpolationMode: 1
// CHECK-NEXT:   OutputStream: 0
// CHECK-NEXT:   ComponentType: 2
// CHECK-NEXT:   DynamicIndexMask: 0
// CHECK-NEXT: PSVSignatureElement:
// CHECK-NEXT:   SemanticName:
// CHECK-NEXT:   SemanticIndex: 0
// CHECK-NEXT:   IsAllocated: 0
// CHECK-NEXT:   StartRow: -1
// CHECK-NEXT:   StartCol: -1
// CHECK-NEXT:   Rows: 1
// CHECK-NEXT:   Cols: 1
// CHECK-NEXT:   SemanticKind: CullPrimitive
// CHECK-NEXT:   InterpolationMode: 1
// CHECK-NEXT:   OutputStream: 0
// CHECK-NEXT:   ComponentType: 1
// CHECK-NEXT:   DynamicIndexMask: 0
// CHECK-NEXT: Outputs affected by ViewID as a bitmask for stream 0:
// CHECK-NEXT:    ViewID influencing Outputs[0] : 0  1  2  3  4  8  12  16
// CHECK-NEXT: PCOutputs affected by ViewID as a bitmask:
// CHECK-NEXT:    ViewID influencing PCOutputs :  3
// CHECK-NEXT: Outputs affected by inputs as a table of bitmasks for stream 0:
// CHECK-NEXT: Inputs contributing to computation of Outputs[0]:  None

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
      op.ormaln = mpl.ormaln + vid;
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
