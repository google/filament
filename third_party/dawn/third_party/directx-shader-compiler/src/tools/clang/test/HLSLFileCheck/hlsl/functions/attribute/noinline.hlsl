// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

// Make sure noinline is present
// CHECK: noinline

[noinline]
float foo() {
    return 42;
}
