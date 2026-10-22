// RUN: %dxc -T ms_6_5 -E main -fcgl  %s -spirv | FileCheck %s

struct MeshOutput {
    float4               PositionCS  : SV_POSITION;
// CHECK: OpDecorate %out_var_VERTEX_INDEX Flat
    nointerpolation uint VertexIndex : VERTEX_INDEX;
};

[outputtopology("triangle")]
[numthreads(128, 1, 1)]
void main(
                 uint       gtid : SV_GroupThreadID,
                 uint       gid  : SV_GroupID,
    out indices  uint3      triangles[128],
    out vertices MeshOutput vertices[64]) {}
