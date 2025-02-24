// RUN: not %dxc -T ms_6_5 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

// CHECK:  17:36: error: only one object with 'vertices' modifier is allowed

struct MeshPerVertex {
    float4 position : SV_Position;
};

#define MAX_VERT 64
#define MAX_PRIM 81
#define NUM_THREADS 128

[outputtopology("line")]
[numthreads(NUM_THREADS, 1, 1)]
void main(
        out vertices MeshPerVertex verts[MAX_VERT],
        out vertices MeshPerVertex verts2[MAX_VERT],
        out indices uint2 primitiveInd[MAX_PRIM])
{
}
