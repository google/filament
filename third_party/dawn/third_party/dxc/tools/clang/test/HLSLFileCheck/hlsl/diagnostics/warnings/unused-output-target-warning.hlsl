// RUN: %dxc -T ps_6_0 %s | FileCheck -input-file=stderr %s

// CHECK-NOT: warning: Declared output SV_Target0 not fully written in shader.
// CHECK: warning: Declared output SV_Target1 not fully written in shader.
// CHECK: warning: Declared output SV_Target2 not fully written in shader.
// CHECK: warning: Declared output SV_Target3 not fully written in shader.

void main(out float4 outRT0 : SV_Target0,
    out float4 outRT1 : SV_Target1,
    out float4 outRT2 : SV_Target2,
    out float4 outRT3 : SV_Target3) {

  outRT0 = 0.0f;
  outRT1.x = 0.0f;
  outRT2.zw = 0.0f;
  // outRT3 = 0.0f;
}
