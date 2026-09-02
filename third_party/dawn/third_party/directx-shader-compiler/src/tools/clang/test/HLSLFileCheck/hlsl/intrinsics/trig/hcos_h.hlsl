// RUN: %dxc -Emain -Tps_6_0 %s | %opt -S -hlsl-dxil-expand-trig-intrinsics | %FileCheck %s

// Make sure the expansion works for half.
// Only checking for for minimal expansion here, full check is done for float case.

// CHECK: fmul fast half %{{.*}}, 0xH3DC5


[RootSignature("")]
min16float main(min16float x : A) : SV_Target {
    return cosh(x);
}