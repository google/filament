// RUN: %dxc -T cs_6_0 %s | FileCheck %s

// Validate that self-assignment of a static struct instance that is not
// referenced does not crash the compiler. This was resulting in an ASAN
// use-after-free in ScalarReplAggregatesHLSL because DeleteMemcpy would
// attempt to delete both source and target, even if both were the same.
// CHECK: define void @main() {
// CHECK-NEXT:   ret void
// CHECK-NEXT: }

struct MyStruct {
  int m0;
};

static MyStruct s;

void foo() {
  s = s;
}

[numthreads(1, 1, 1)]
void main() {
  foo();
}
