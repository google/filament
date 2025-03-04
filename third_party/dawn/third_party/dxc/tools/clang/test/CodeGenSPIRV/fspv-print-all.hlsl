// RUN: %dxc -T ps_6_0 -E main -spirv -fspv-print-all -fcgl  %s -spirv | FileCheck %s

// We expect the whole module to be printed multiple times, but we cannot check
// that here as stderr contents do not appear as test diagnostics. Instead,
// just confirm that the file compiles fine with `-fspv-print-all` enabled.

// CHECK: OpEntryPoint Fragment %main

struct Input
{
  float4 color : COLOR;
};

float4 main(Input input) : SV_TARGET
{
  return input.color;
}
