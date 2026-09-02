// RUN: %dxc -E main -T ms_6_5 %s | FileCheck %s

// Regression test for a crash where the primitive signature was used when looking
// up vertex output signatures, so if the vertices struct had more signature elements
// than the primitives struct, it would crash

// CHECK: @main

struct Vertex { float4 pos : SV_POSITION; };
struct Primitive {};

[outputtopology("point")]
[numthreads(2,2,1)]
void main(
    out vertices Vertex verts[1],
    out primitives Primitive prims[1],
    out indices uint3 idx[1])
{
     verts = (Vertex[1])0;
     SetMeshOutputCounts(0, 0);
}