// RUN: %dxc -E main -T cs_6_0 -HV 2016 %s | FileCheck %s

// Make sure we still produced second loop, and unrolled outer and inner loops,
// producing two stores, and no more.

// CHECK: void @main()
// CHECK-NOT: call void @dx.op.bufferStore.i32(i32 69
// CHECK: phi i32

// CHECK: call void @dx.op.bufferStore.i32(i32 69
// CHECK: call void @dx.op.bufferStore.i32(i32 69
// CHECK-NOT: call void @dx.op.bufferStore.i32(i32 69

// CHECK: !llvm.loop
// CHECK-NOT: call void @dx.op.bufferStore.i32(i32 69
// CHECK: ret void

struct Foo {
  bool b;
};

RWStructuredBuffer<Foo> sb;

[numthreads(1,1,1)]
void main() {
  Foo foo;
  bool b = true;

  [unroll]
  for (int i = 2; i >= 0; i--) {
    foo.b = b;
    [loop]
    for (int j = 0; j < i + 1; j++) {
      [unroll]
      for (int k = 0; k < 2; k++) {
        if (foo.b) {
          sb[0] = foo;
        }
      }
    }
    b = false;
  }
}
