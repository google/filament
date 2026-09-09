// Validate that the offsets for a min16float vector is correct if 16-bit types are enabled
// RUN: %dxc -T cs_6_5 -Od -Zi -Qembed_debug -enable-16bit-types %s | FileCheck %s

// CHECK-NOT: {{.*}}!DIDerivedType{{.*}}name: "x"{{.*}}offset: {{.*}}
// CHECK: {{.*}}!DIDerivedType{{.*}}name: "y"{{.*}}offset: 16{{.*}}
// CHECK: {{.*}}!DIDerivedType{{.*}}name: "z"{{.*}}offset: 32{{.*}}
// CHECK: {{.*}}!DIDerivedType{{.*}}name: "w"{{.*}}offset: 48{{.*}}

RWStructuredBuffer<int> UAV : register(u0);

[numthreads(1, 1, 1)] 
void main() {
    min16float4 vector;
    vector.x = UAV[0];
    vector.y = UAV[1];
    vector.z = UAV[2];
    vector.w = UAV[3];
    UAV[16] = vector.x + vector.y + vector.z + vector.w;
}