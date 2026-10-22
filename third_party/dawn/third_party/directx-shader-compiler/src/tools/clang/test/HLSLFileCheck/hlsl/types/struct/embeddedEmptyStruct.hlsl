// RUN: %dxc -E main -T vs_6_2 %s | FileCheck %s

struct s0 {};
struct s1 { s0 a; uint b; };

// CHECK: ret
s1 main() : OUT { s1 s; return s; }

