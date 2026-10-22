// RUN: %dxc -T cs_6_10 -spirv %s | FileCheck %s

// CHECK: OpExtension "SPV_KHR_non_semantic_info"
// CHECK: OpExtInstImport "NonSemantic.DebugBreak"
// CHECK: OpExtInst %void {{%[0-9]+}} 1

[numthreads(8, 8, 1)]
void main(uint3 threadId : SV_DispatchThreadID) {
    if (threadId.x == 0) {
        DebugBreak();
    }
}
