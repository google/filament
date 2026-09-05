// RUN: %dxc -T ms_6_6 %s | FileCheck %s

// For https://github.com/microsoft/DirectXShaderCompiler/issues/6940
// Ensure the shader compiles when the semantic is directly on the parameter.
// Only one semantic index should be created.

// CHECK:; SV_Position              0   xyzw        0      POS   float   xyzw
// CHECK-NOT:; SV_Position              1   xyzw        0      POS   float   xyzw

// CHECK:; A                        0   xyzw        0     NONE    uint
// CHECK-NOT: ; A                        1   xyzw        0     NONE    uint

#define GROUP_SIZE 30

cbuffer Constant : register(b0)
{
    uint numPrims;
}
static const uint numVerts = 3;

[RootSignature("RootConstants(num32BitConstants=1, b0)")]
[numthreads(GROUP_SIZE, 1, 1)]
[OutputTopology("triangle")]
void main(
    uint gtid : SV_GroupThreadID,
    out indices uint3 tris[GROUP_SIZE],
    out vertices float4 verts[GROUP_SIZE] : SV_Position,
    out  primitives uint4 t[GROUP_SIZE] : A
)
{
    SetMeshOutputCounts(numVerts, numPrims);

    if (gtid < numVerts)
    {
        verts[gtid] = 0;
    }

    if (gtid < numPrims)
    {
        tris[gtid] = uint3(0, 1, 2);
    }
}
