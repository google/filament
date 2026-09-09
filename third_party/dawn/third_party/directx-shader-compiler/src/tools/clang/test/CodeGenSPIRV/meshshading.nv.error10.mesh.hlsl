// RUN: not %dxc -T ms_6_5 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

// CHECK:  19:6: error: array size of primitives object should match 'indices' object

struct MeshPerVertex {
    float4 position : SV_Position;
};

struct MeshPerPrimitive {
    float3 userPrimAttr : PRIM_USER;
};

#define MAX_VERT 64
#define MAX_PRIM 81
#define NUM_THREADS 128

[outputtopology("line")]
[numthreads(NUM_THREADS, 1, 1)]
void main(
        out vertices MeshPerVertex verts[MAX_VERT],
        out indices uint2 primitiveInd[MAX_PRIM],
        out primitives MeshPerPrimitive prims[MAX_PRIM + 1])
{
}
