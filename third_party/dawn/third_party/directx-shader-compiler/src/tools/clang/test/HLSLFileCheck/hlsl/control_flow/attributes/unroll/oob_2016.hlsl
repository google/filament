// RUN: %dxc -Od -E main -T ps_6_0 -HV 2016 %s | FileCheck %s
// CHECK: call i32 @dx.op.bufferUpdateCounter
// CHECK: call i32 @dx.op.bufferUpdateCounter
// CHECK: call i32 @dx.op.bufferUpdateCounter
// CHECK: call i32 @dx.op.bufferUpdateCounter
// CHECK: call i32 @dx.op.bufferUpdateCounter
// CHECK: call i32 @dx.op.bufferUpdateCounter
// CHECK-NOT: call i32 @dx.op.bufferUpdateCounter

AppendStructuredBuffer<float> buf0;
AppendStructuredBuffer<float> buf1;
AppendStructuredBuffer<float> buf2;
AppendStructuredBuffer<float> buf3;

uint g_cond;

float main() : SV_Target {
  AppendStructuredBuffer<float> buffs[4] = {
    buf0, buf1, buf2, buf3,
  };
  
  float result = 0;
  [unroll]
  for (int j = -1; j < 4+1; j++) {
    if (j == g_cond) {
      buffs[j].Append(g_cond);
      break;
    }
    result += 1;
  }
  return result;
}

