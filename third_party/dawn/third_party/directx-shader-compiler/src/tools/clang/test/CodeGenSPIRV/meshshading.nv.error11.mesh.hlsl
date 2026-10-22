// RUN: not %dxc -T ms_6_5 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

// CHECK:  15:36: error: expected 1D array of indices/vertices/primitives object

struct MeshPerVertex {
    float4 position : SV_Position;
};

#define MAX_PRIM 81
#define NUM_THREADS 128

[outputtopology("line")]
[numthreads(NUM_THREADS, 1, 1)]
void main(
        out vertices MeshPerVertex verts,
        out indices uint2 primitiveInd[MAX_PRIM])
{
}
