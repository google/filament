// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s
// CHECK: call i32 @dx.op.bufferUpdateCounter
// CHECK: call i32 @dx.op.bufferUpdateCounter
// CHECK: call i32 @dx.op.bufferUpdateCounter
// CHECK: call i32 @dx.op.bufferUpdateCounter

// CHECK: call i32 @dx.op.bufferUpdateCounter
// CHECK: call i32 @dx.op.bufferUpdateCounter
// CHECK: call i32 @dx.op.bufferUpdateCounter
// CHECK: call i32 @dx.op.bufferUpdateCounter

// CHECK: call i32 @dx.op.bufferUpdateCounter
// CHECK: call i32 @dx.op.bufferUpdateCounter
// CHECK: call i32 @dx.op.bufferUpdateCounter
// CHECK: call i32 @dx.op.bufferUpdateCounter

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
uint g_cond2;

float routine(float value) {
  AppendStructuredBuffer<float4> buffers[] = { buf0, buf1, buf2, buf3, };
  float ret = 0;
  [unroll]
  for (uint k = 0; k < 4; k++) {
    ret += 15;
    if (g_cond == k) {
      buffers[k].Append(value);
      return ret;
    }
  }
  return ret+1;
}

float main(float3 a : A, float3 b : B) : SV_Target {

  float ret = 0;
  [unroll]
  for (uint l = 0; l < 1; l++) {
    [unroll]
    for (uint i = 0; i < 4; i++) {

      [loop]
      for (uint j = 0; j < 4; j++) {
        ret += routine(j);
        ret++;
      }

      ret--;
    }
  }

  return ret;
}

