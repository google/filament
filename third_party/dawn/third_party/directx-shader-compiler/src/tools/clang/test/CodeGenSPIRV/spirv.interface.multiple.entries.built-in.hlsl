// RUN: %dxc -T lib_6_6 -fcgl  %s -spirv | FileCheck %s

// CHECK: OpEntryPoint GLCompute %bar "bar" %gl_GlobalInvocationID
// CHECK: OpEntryPoint GLCompute %foo "foo" %gl_GlobalInvocationID

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
