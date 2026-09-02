// RUN: %dxc -T ps_6_0 -E main -fcgl %s -spirv | FileCheck %s

// CHECK:      OpExecutionMode %main DepthLess
// CHECK-NEXT: OpExecutionMode %main PostDepthCoverage
[[vk::spvexecutionmode(4446),vk::spvexecutionmode(15)]]
void main() {}
