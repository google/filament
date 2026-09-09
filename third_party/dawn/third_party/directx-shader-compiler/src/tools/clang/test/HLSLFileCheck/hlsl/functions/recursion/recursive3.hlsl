// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: error: recursive functions are not allowed: function 'main' calls recursive function 'test_ret'

float test_ret()
{
    return test_ret();
}

float4 main(float a : A, float b:B) : SV_TARGET
{
  return test_ret();
}

