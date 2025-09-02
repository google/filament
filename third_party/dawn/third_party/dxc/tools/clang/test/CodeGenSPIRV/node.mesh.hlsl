// RUN: %dxc -spirv -T lib_6_9 -fspv-target-env=vulkan1.3 %s | FileCheck %s
// XFAIL: *
// disabled until mesh nodes are implemented

// Test loading of node input and funneling into mesh outputs
// Essentially an end-to-end mesh node test.


RWBuffer<float> buf0;

#define MAX_VERT 32
#define MAX_PRIM 16

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
};

struct MeshPayload {
    float normal;
    float malnor;
    float alnorm;
    float ormaln;
    int layer[6];
};

groupshared float gsMem[MAX_PRIM];

[Shader("node")]
[NodeLaunch("mesh")]
[outputtopology("triangle")]
[numthreads(128, 1, 1)]
[NodeDispatchGrid(64,1,1)]
void node_setmeshoutputcounts(DispatchNodeInputRecord<MeshPayload> mpl,
            out indices uint3 primIndices[MAX_PRIM],
            out vertices MeshPerVertex verts[MAX_VERT],
            out primitives MeshPerPrimitive prims[MAX_PRIM],
            in uint tig : SV_GroupIndex) {
  SetMeshOutputCounts(32, 16);

  // create mpl

  MeshPerVertex ov;
  ov.position = float4(14.0,15.0,16.0,17.0);
  ov.color[0] = 14.0;
  ov.color[1] = 15.0;
  ov.color[2] = 16.0;
  ov.color[3] = 17.0;

  if (tig % 3) {
    primIndices[tig / 3] = uint3(tig, tig + 1, tig + 2);

    MeshPerPrimitive op;
    op.normal = mpl.Get().normal;
    op.malnor = gsMem[tig / 3 + 1];
    op.alnorm = mpl.Get().alnorm;
    op.ormaln = mpl.Get().ormaln;
    op.layer[0] = mpl.Get().layer[0];
    op.layer[1] = mpl.Get().layer[1];
    op.layer[2] = mpl.Get().layer[2];
    op.layer[3] = mpl.Get().layer[3];
    op.layer[4] = mpl.Get().layer[4];
    op.layer[5] = mpl.Get().layer[5];

    gsMem[tig / 3] = op.normal;
    prims[tig / 3] = op;
  }
  verts[tig] = ov;
}

// CHECK: OpEntryPoint MeshExt [[ENTRY:%[^ ]*]]
// CHECK-DAG: OpExecutionMode [[ENTRY]] OutputVertices 32
// CHECK-DAG: OpExecutionMode [[ENTRY]] OutputPrimitivesNV 16
// CHECK-DAG: OpExecutionMode [[ENTRY]] OutputTrianglesNV
// CHECK: [[UINT:%[^ ]*]] = OpTypeInt 32 0
// CHECK-DAG: [[U16:%[^ ]*]] = OpConstant [[UINT]] 16
// CHECK-DAG: [[U32:%[^ ]*]] = OpConstant [[UINT]] 32
// CHECK: [[ENTRY]] = OpFunction
// CHECK: OpSetMeshOutputsEXT [[U32]] [[U16]]
// CHECK: OpFunctionEnd
