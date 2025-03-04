// RUN: not %dxc -T ms_6_5 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

// CHECK:  18:19: error: invalid usage of semantic 'USER_IN' in shader profile ms

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
        out indices uint2 primitiveInd[MAX_PRIM],
        in float3 userAttrIn : USER_IN
        )
{
}
