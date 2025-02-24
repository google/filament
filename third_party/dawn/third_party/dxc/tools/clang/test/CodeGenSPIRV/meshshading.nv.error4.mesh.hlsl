// RUN: not %dxc -T ms_6_5 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

// CHECK: 22:9: error: 'payload' object must be an in parameter

struct MeshPerVertex {
    float4 position : SV_Position;
};

struct MeshPayload {
    float4 pos;
};

#define MAX_VERT 64
#define MAX_PRIM 81
#define NUM_THREADS 128

[outputtopology("triangle")]
[numthreads(NUM_THREADS, 1, 1)]
void main(
        out vertices MeshPerVertex verts[MAX_VERT],
        out indices uint3 primitiveInd[MAX_PRIM],
        out payload MeshPayload pld,
        in uint tig : SV_GroupIndex)
{
}
