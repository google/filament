// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: invalid semantic 'SV_Position'

float4 main(float4 p) : SV_Position {
  return 0;
}
