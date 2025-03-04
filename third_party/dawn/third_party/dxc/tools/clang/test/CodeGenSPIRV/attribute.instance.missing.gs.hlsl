// RUN: %dxc -T gs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpExecutionMode %main Invocations 1

struct S { float4 val : VAL; };

[maxvertexcount(2)]
void main(point in uint id[1] : VertexID, inout LineStream<S> outData) {
}
