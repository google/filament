// RUN: %dxc -E main -T vs_6_0 -O0 %s | FileCheck %s

// Ensure that bools are converted from/to their memory representation when loaded/stored
// in local variables.

int main(int i : I) : OUT
{
    // CHECK: icmp eq i32 {{.*}}, 42
    bool s = i == 42;
    // CHECK: icmp eq i32 {{.*}}, 42
    // CHECK: zext i1 {{.*}} to i32
    bool1 v = i == 42;
    // CHECK: icmp eq i32 {{.*}}, 42
    // CHECK: zext i1 {{.*}} to i32
    bool1x1 m = i == 42;
    // CHECK: icmp eq i32 {{.*}}, 42
    // CHECK: zext i1 {{.*}} to i32
    bool sa[1] = { i == 42 };
    // CHECK: icmp eq i32 {{.*}}, 42
    // CHECK: zext i1 {{.*}} to i32
    bool1 va[1] = { i == 42 };
    // CHECK: icmp eq i32 {{.*}}, 42
    // CHECK: zext i1 {{.*}} to i32
    bool1x1 ma[1] = { i == 42 };

    // Used to check icmp ne i32 {{.*}}, 0
    // but since variable "s" was never stored
    // to memory, it stayed as an i1 value,
    // so no need to icmp that to 0.
    return (s
        // CHECK: icmp ne i32 {{.*}}, 0
        && v.x
        // CHECK: icmp ne i32 {{.*}}, 0
        && m._11
        // CHECK: icmp ne i32 {{.*}}, 0
        && sa[0]
        // CHECK: icmp ne i32 {{.*}}, 0
        && va[0].x
        // CHECK: icmp ne i32 {{.*}}, 0
        && ma[0]._11) ? 1 : 2;
}
