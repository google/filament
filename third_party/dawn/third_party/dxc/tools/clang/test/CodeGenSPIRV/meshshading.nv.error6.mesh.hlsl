// RUN: not %dxc -T ms_6_5 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

// CHECK: 17:9: error: 'indices' object must be an out parameter

struct MeshPerVertex {
    float4 position : SV_Position;
};

#define MAX_VERT 64
#define MAX_PRIM 81
#define NUM_THREADS 128

[outputtopology("triangle")]
[numthreads(NUM_THREADS, 1, 1)]
void main(
        out vertices MeshPerVertex verts[MAX_VERT],
        in indices uint3 primitiveInd[MAX_PRIM])
{
}
