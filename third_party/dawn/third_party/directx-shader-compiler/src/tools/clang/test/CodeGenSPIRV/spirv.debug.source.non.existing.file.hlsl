// RUN: %dxc -T ps_6_0 -E main -Zi -fcgl  %s -spirv | FileCheck %s

// CHECK: OpString "non_existing_file.txt"

#line 1 "non_existing_file.txt"

struct PSInput
{
  float4 color : COLOR;
};

float4 main(PSInput input) : SV_TARGET
{
  return input.color;
}
