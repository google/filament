// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s
// CHECK: @main

AppendStructuredBuffer<float4> buf0;
AppendStructuredBuffer<float4> buf1;
AppendStructuredBuffer<float4> buf2;
AppendStructuredBuffer<float4> buf3;
uint g_cond;

float main() : SV_Target {

  AppendStructuredBuffer<float4> buffers[] = { buf0, buf1, buf2, buf3, };

  float ret = 0;
  [unroll]
  for (uint i = 0; i < 4; i++) {
    [unroll]
    for (uint j = 0; j < 4; j++) {
      ret++;
      if (g_cond == j) {
        buffers[j].Append(i);
        return ret;
      }
    }
    ret--;
  }

  return ret;
}

