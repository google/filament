// RUN: %dxc -T gs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpExecutionMode %main InputLinesAdjacency

struct S { float4 val : VAL; };

[maxvertexcount(3)]
void main(lineadj in uint id[4] : VertexID, inout LineStream<S> outData) {
}
