// RUN: %dxc /Tps_6_2 /Emain  %s | FileCheck %s
// CHECK: define void @main()
// CHECK: %{{[a-z0-9]+.*[a-z0-9]*}} = frem fast float %{{[a-z0-9]+.*[a-z0-9]*}}, 1.000000e+01
// CHECK: %{{[a-z0-9]+.*[a-z0-9]*}} = fdiv fast float %{{[a-z0-9]+.*[a-z0-9]*}}, %{{[a-z0-9]+.*[a-z0-9]*}}
// CHECK: entry

float foo(float v0, float v1) { return v0 / v1; }
half foo(half v0, half v1) { return v0 * v1; }
min16float foo(min16float v0, min16float v1) { return v0 + v1; }
min10float foo(min10float v0, min10float v1) { return v0 - v1; }

[RootSignature("")]
float main(float vf
                                : A, half vh
                                : B, min16float vm16
                                : C, min10float vm10
                                : D) : SV_Target {
  return foo(vf, vf % 10.0);
}