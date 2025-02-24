// RUN: %dxc -Emain -Tps_6_0 %s | %opt -S -hlsl-dxil-constantColor,constant-red=2.5,constant-green=8,constant-blue=7,constant-alpha=6 | %FileCheck %s

// Added override output color:
// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 2)

[RootSignature("")]
unsigned int main() : SV_Target {
    return 0;
}
