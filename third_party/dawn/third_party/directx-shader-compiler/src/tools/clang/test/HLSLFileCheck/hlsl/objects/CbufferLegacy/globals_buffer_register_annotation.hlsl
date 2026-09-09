// RUN: %dxc -T vs_6_0 -E main %s | FileCheck %s

// Test that register annotations on globals constants affect the offset.

// CHECK: ; int x; ; Offset: 16
// CHECK: ; int y; ; Offset:  0

int x : register(c1);
int y : register(c0);

int2 main() : OUT { return int2(x, y); }