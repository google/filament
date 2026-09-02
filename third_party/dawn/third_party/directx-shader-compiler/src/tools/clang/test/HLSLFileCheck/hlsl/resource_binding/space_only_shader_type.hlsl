// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Test space-only register annotations with shader type specifier.

// CHECK: buf texture u32 buf T0 t0,space2 1
Buffer<uint> buf : register(ps, space1) : register(vs, space2);

uint main() : OUT { return buf[0]; }