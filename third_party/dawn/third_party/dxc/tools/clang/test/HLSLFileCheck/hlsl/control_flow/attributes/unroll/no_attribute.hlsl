// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s
// CHECK-NOT: @main

// Without [unroll] attribute, the special unroll
// routine is not done of the loop, and the resource
// fail to get mapped.

AppendStructuredBuffer<float4> buf0;
AppendStructuredBuffer<float4> buf1;
AppendStructuredBuffer<float4> buf2;
AppendStructuredBuffer<float4> buf3;
uint g_cond;

float main() : SV_Target {

  AppendStructuredBuffer<float4> buffers[] = { buf0, buf1, buf2, buf3, };

  for (uint j = 0; j < 4; j++) {
    if (g_cond == j) {
      buffers[j].Append(1);
      return 10;
    }
  }

  return 0;
}

