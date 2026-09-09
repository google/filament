// RUN: %dxc -Od -E main -T ps_6_0 %s | FileCheck %s
// CHECK: @main
// CHECK: call i32 @dx.op.bufferUpdateCounter
// CHECK: call i32 @dx.op.bufferUpdateCounter
// CHECK: call i32 @dx.op.bufferUpdateCounter
// CHECK: call i32 @dx.op.bufferUpdateCounter
// CHECK-NOT: call i32 @dx.op.bufferUpdateCounter

AppendStructuredBuffer<float4> buf0;
AppendStructuredBuffer<float4> buf1;
AppendStructuredBuffer<float4> buf2;
AppendStructuredBuffer<float4> buf3;
uint g_cond;

struct Params {
  int foo;
};

float f(Params p) {

  AppendStructuredBuffer<float4> buffers[2][2] = { buf0, buf1, buf2, buf3, };

  [unroll]
  for (uint j = 0; j < p.foo; j++) {
    if (g_cond == j) {
      buffers[j/2][j%2].Append(1);
      return 10;
    }
  }

  return 0;
}

float main() : SV_Target {
  Params p;
  p.foo = 4;

  return f(p);
}

