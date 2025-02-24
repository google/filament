// RUN: %dxc -E main -T ms_6_5 %s | FileCheck %s

#define MAX_VERT 32
#define MAX_PRIM 16
#define NUM_THREADS 32
struct MeshPerVertex {
    float4 position : SV_Position;
    float4 color : COLOR;
};

struct MeshPerPrimitive {
// CHECK: error: Mesh shader's primitive outputs' interpolation mode must be constant or undefined.
  linear float normal : NORMAL;
// CHECK: error: Mesh shader's primitive outputs' interpolation mode must be constant or undefined.
  centroid float malnor : MALNOR;
// CHECK: error: Mesh shader's primitive outputs' interpolation mode must be constant or undefined.
    sample float alnorm : ALNORM;
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
        ov.color = float4(4.0, 5.0, 6.0, 7.0);
    } else {
        ov.position = float4(14.0,15.0,16.0,17.0);
        ov.color = float4(14.0, 15.0, 16.0, 17.0);
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
