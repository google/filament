// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: call void @dx.op.storeOutput.i32{{.*}}, i32 -1

[RootSignature("")]
int main() : SV_Target {
    return firstbithigh((int)0xffffffff);
}