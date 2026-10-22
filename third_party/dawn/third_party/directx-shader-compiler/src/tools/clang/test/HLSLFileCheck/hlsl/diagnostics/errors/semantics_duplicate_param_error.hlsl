// RUN: %dxc -T ps_6_0 -E main %s | FileCheck %s

// Tests that we prevent multiple inputs/outputs from having the same semantics.

// CHECK: validation errors
// CHECK: Semantic 'TEXCOORD' overlap at 0

float main(float u : TEXCOORD0, float v : TEXCOORD0) : SV_Target { return 1; }