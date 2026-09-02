// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: icmp ne i32 {{.*}}, 0
// CHECK: icmp ne i32 {{.*}}, 0
// CHECK: icmp ne i32 {{.*}}, 0
// CHECK: icmp ne i32 {{.*}}, 0

struct Struct { bool2x2 mat; };
ConstantBuffer<Struct> cb;
bool2x2 main() : B { return cb.mat; }