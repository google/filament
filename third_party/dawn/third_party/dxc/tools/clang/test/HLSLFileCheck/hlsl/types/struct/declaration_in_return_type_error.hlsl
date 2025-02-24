// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Tests that struct declarations cannot also declare functions.
// Note that FXC allows this

// CHECK: error: {{.*}} cannot be defined in the result type of a function

struct Struct { int x; };
struct { int x; } main() : OUT
{
    Struct result;
    return result;
}