// RUN: %dxc -E main -T vs_6_0 -O0 %s -fcgl | FileCheck %s

// Ensure that bools are converted from/to their memory representation when loaded/stored
// in local variables.

// Local variables should never be i1s
// CHECK-NOT: alloca {{.*}}i1

int main(int i : I) : OUT
{
    // CHECK: alloca i32
    // CHECK: icmp eq i32 {{.*}}, 42
    // CHECK: zext i1 {{.*}} to i32
    bool s = i == 42;
    bool1 v = i == 42;
    bool1x1 m = i == 42;
    bool sa[1] = { i == 42 };
    bool1 va[1] = { i == 42 };
    bool1x1 ma[1] = { i == 42 };

    return (s
        && v.x
        && m._11
        && sa[0]
        && va[0].x
        && ma[0]._11) ? 1 : 2;
}
