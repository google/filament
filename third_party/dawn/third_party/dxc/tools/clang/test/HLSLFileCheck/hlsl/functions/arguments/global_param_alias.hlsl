// RUN: %dxc -E main -T ps_6_0  %s | FileCheck %s

// Make sure static global a not alias with local t.

// store io before call foo. Value should be 0.
// CHECK:  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %{{.*}}, i32 0, i32 0, i32 0, i32 undef, i32 undef, i32 undef, i8 1)
// CHECK:  call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle %{{.*}}, i32 0, i32 4, float 0.000000e+00, float undef, float undef, float undef, i8 1)
// CHECK:  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %{{.*}}, i32 0, i32 8, i32 0, i32 undef, i32 undef, i32 undef, i8 1)

// store io after ++ in foo and after call foo. Value should be 1.
// CHECK:  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %{{.*}}, i32 1, i32 0, i32 1, i32 undef, i32 undef, i32 undef, i8 1)
// CHECK:  call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle %{{.*}}, i32 1, i32 4, float 1.000000e+00, float undef, float undef, float undef, i8 1)
// CHECK:  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %{{.*}}, i32 1, i32 8, i32 1, i32 undef, i32 undef, i32 undef, i8 1)
// CHECK:  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %{{.*}}, i32 1, i32 0, i32 1, i32 undef, i32 undef, i32 undef, i8 1)
// CHECK:  call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle %{{.*}}, i32 1, i32 4, float 1.000000e+00, float undef, float undef, float undef, i8 1)
// CHECK:  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %{{.*}}, i32 1, i32 8, i32 1, i32 undef, i32 undef, i32 undef, i8 1)

// sore a after bar. Value should be -1.
// CHECK:  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %{{.*}}, i32 -1, i32 0, i32 -1, i32 undef, i32 undef, i32 undef, i8 1)
// CHECK:  call void @dx.op.bufferStore.f32(i32 69, %dx.types.Handle %{{.*}}, i32 -1, i32 4, float -1.000000e+00, float undef, float undef, float undef, i8 1)
// CHECK:  call void @dx.op.bufferStore.i32(i32 69, %dx.types.Handle %{{.*}}, i32 -1, i32 8, i32 -1, i32 undef, i32 undef, i32 undef, i8 1)

// Make sure return 3.
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 0, float 3.000000e+00)

struct ST {
   int a;
   float b;
   uint c;
};

static ST a;


const static ST b = {0, 0, 0};

RWStructuredBuffer<ST> u;

void foo(inout ST io) {
  io.a++;
  io.b++;
  io.c++;
  u[io.a] = io;
}

void bar(inout ST io) {
  a.a--;
  a.b--;
  a.c--;
  u[io.a] = io;
  foo(io);
  u[io.a] = io;
}

float main() : SV_Target {
  a = b;
  ST t = a;
  bar(t);

  u[a.a] = a;
  return t.a + t.b + t.c;
}