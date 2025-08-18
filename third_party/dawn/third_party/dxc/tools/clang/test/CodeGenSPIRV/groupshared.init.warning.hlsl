// RUN: %dxc -T cs_6_0 -E main -spirv %s 2>&1 | FileCheck %s

groupshared uint testing = 0;

[numthreads(64, 1, 1)]
void main(uint local_thread_id_flat : SV_GroupIndex) {
    
    InterlockedAdd(testing, 1);
    GroupMemoryBarrierWithGroupSync();
    
    if (local_thread_id_flat == 0) {
        if (testing > 64) {
            printf("testing is %u wtf", testing);
        }
    }
}

// CHECK: warning: Initializer of external global will be ignored
// CHECK-NEXT: groupshared uint testing = 0;