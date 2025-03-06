// RUN: %dxc -E main -T ms_6_5 %s | FileCheck %s

// CHECK: dx.op.getMeshPayload.struct.MeshPayload
// CHECK: dx.op.setMeshOutputCounts(i32 168, i32 30, i32 10)
// CHECK: dx.op.emitIndices
// CHECK: dx.op.storeVertexOutput
// CHECK: dx.op.storePrimitiveOutput

#define MAX_VERT 30
#define MAX_PRIM 10
#define NUM_THREADS 32
struct MeshPerVertex {
    float4 position : SV_Position;
    float4 color : COLOR;
};

struct MeshPerPrimitive {
    float normal : NORMAL;
    float malnor : MALNOR;
    float alnorm : ALNORM;
    float ormaln : ORMALN;
    int4 layer0 : LAYER0;
    int2 layer1 : LAYER1;
};

struct MeshPayload {
    float normal;
    float malnor;
    float alnorm;
    float ormaln;
    int layer[6];
};

groupshared float gsMem[MAX_PRIM];

cbuffer CB1 : register(b1, space2)
{
  uint idx;
}

[RootSignature("CBV(b1, space=2, visibility=SHADER_VISIBILITY_MESH)")]
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
    if (tig < MAX_PRIM) {
      uint3 indices = (tig * 3) + uint3(0, 1, 2);
      primIndices[tig] = indices;
      MeshPerPrimitive op;
      op.normal = mpl.normal;
      gsMem[tig] = op.normal;
      op.malnor = gsMem[(tig + idx) % MAX_PRIM];
      op.alnorm = mpl.alnorm;
      op.ormaln = mpl.ormaln;
      op.layer0 = int4(mpl.layer[0], mpl.layer[1], mpl.layer[2], mpl.layer[3]);
      op.layer1 = int2(mpl.layer[4], mpl.layer[5]);
      prims[tig] = op;
    }
    if (tig < MAX_VERT) {
      MeshPerVertex ov;
      if (vid % 2) {
          ov.position = float4(0.0, 1.0, 2.0, 3.0);
          ov.color = float4(4.0, 5.0, 6.0, 7.0);
      } else {
          ov.position = float4(10.0, 11.0, 12.0, 13.0);
          ov.color = float4(14.0, 15.0, 16.0, 17.0);
      }
      verts[tig] = ov;
    }
}
