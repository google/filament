// RUN: %dxc -E main -T ps_6_0 -fcgl %s | %opt -scalarrepl-param-hlsl -dxilgen -S | FileCheck %s

// CHECK: dx.op.storeOutput.f32

float4 main() : SV_Target {
  return 0;
}
