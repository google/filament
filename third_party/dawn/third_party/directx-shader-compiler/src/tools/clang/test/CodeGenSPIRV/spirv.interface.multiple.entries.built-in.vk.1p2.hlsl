// RUN: %dxc -T lib_6_6 -fspv-target-env=vulkan1.2 -fcgl  %s -spirv | FileCheck %s

// CHECK: OpEntryPoint GLCompute %bar "bar"
// CHECK-DAG: %MyBuffer
// CHECK-DAG: %gl_GlobalInvocationID

// CHECK: OpEntryPoint GLCompute %foo "foo"
// CHECK-DAG: %MyBuffer
// CHECK-DAG: %gl_GlobalInvocationID

RWBuffer<uint> MyBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void bar(uint tid : SV_DispatchThreadId) {
    MyBuffer[0] = tid;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void foo(uint tid : SV_DispatchThreadId) {
    MyBuffer[0] = tid;
}
