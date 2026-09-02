// RUN: %dxc -E main -T ms_6_5 %s | FileCheck %s

// CHECK: payload plus output size is greater than 48128

#define MAX_VERT 230
#define MAX_PRIM 93
#define NUM_THREADS 32
struct MeshPerVertex {
    float4 position : SV_Position;
    float color[4] : COLOR;
};

struct MeshPerPrimitive {
    float normal : NORMAL;
    float4 malnor[14] : MALNOR;
    int layer[6] : LAYER;
};

struct MeshPayload {
    float normal;
    float malnor[4091];
    int layer[4];
};

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
      op.malnor[0].x = mpl.malnor[0];
      op.layer[0] = mpl.layer[0];
      op.layer[1] = mpl.layer[1];
      op.layer[2] = mpl.layer[2];
      op.layer[3] = mpl.layer[3];
      op.layer[4] = mpl.layer[3];
      op.layer[5] = mpl.layer[3];
      prims[tig / 3] = op;
    }
    verts[tig] = ov;
}