// RUN: %dxc -T gs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpExecutionMode %main InputTrianglesAdjacency

struct S { float4 val : VAL; };

[maxvertexcount(3)]
void main(triangleadj in uint id[6] : VertexID, inout LineStream<S> outData) {
}
