// RUN: %dxc -T gs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpExecutionMode %main InputLines

struct S { float4 val : VAL; };

[maxvertexcount(3)]
void main(line in uint id[2] : VertexID, inout LineStream<S> outData) {
}
