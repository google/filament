// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpExecutionMode %main EarlyFragmentTests

[earlydepthstencil]
float4 main() : SV_Target { return 1.0; }
