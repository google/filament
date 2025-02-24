// RUN: %dxc /HV 2016 -Od -E main -T ps_6_0 %s | FileCheck %s
// CHECK-DAG: warning: Could not unroll loop.
// CHECK-NOT: -HV 2016
// CHECK-NOT: @main

// Check that the warning doesn't mention HV 2016
// Check that the compilation fails due to unable to
// find the loop bound.

uint g_cond;

AppendStructuredBuffer<float4> buf0;
AppendStructuredBuffer<float4> buf1;
AppendStructuredBuffer<float4> buf2;
AppendStructuredBuffer<float4> buf3;

float main() : SV_Target {

  AppendStructuredBuffer<float4> buffers[] = { buf0, buf1, buf2, buf3, };

  float result = 0;
  [unroll]
  for (uint j = 0; j < g_cond; j++) {
    buffers[j].Append(result);
    result += 1;
  }
  return result;
}

