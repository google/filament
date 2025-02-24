// RUN: not %dxc -T ms_6_5 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

// CHECK: error: mesh entry point must have a valid outputtopology attribute

struct MeshPerVertex {
    float4 position : SV_Position;
};

#define MAX_VERT 64
#define MAX_PRIM 81
#define NUM_THREADS 128

[numthreads(NUM_THREADS, 1, 1)]
void main(
        out vertices MeshPerVertex verts[MAX_VERT],
        out indices uint3 primitiveInd[MAX_PRIM],
        in uint tig : SV_GroupIndex)
{
}
