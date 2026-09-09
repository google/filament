// RUN: not %dxc -T ms_6_5 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

// CHECK: 21:9: error: 'primitives' object must be an out parameter

struct MeshPerVertex {
    float4 position : SV_Position;
};

struct MeshPerPrimitive {
    int primId : SV_PrimitiveID;
};

#define MAX_VERT 64
#define MAX_PRIM 81
#define NUM_THREADS 128

[outputtopology("triangle")]
[numthreads(NUM_THREADS, 1, 1)]
void main(
        out vertices MeshPerVertex verts[MAX_VERT],
        in primitives MeshPerPrimitive prims[MAX_PRIM],
        out indices uint3 primitiveInd[MAX_PRIM])
{
}
