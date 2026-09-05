// RUN: %dxc -T lib_6_3 -auto-binding-space 11 %s | FileCheck %s

struct MyPayload {
  float4 color;
  uint2 pos;
};

// Fine.
[shader("miss")]
void miss_nop( inout MyPayload payload ) {}

// CHECK: error: return type for 'miss' shaders must be void

[shader("miss")]
float miss_ret() { return 1.0; }
