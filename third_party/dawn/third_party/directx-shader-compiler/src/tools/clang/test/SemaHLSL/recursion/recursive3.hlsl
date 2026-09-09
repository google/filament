// RUN: %dxc -E main -T ps_6_0 %s -verify

// expected-error@+2{{recursive functions are not allowed: function 'main' calls recursive function 'test_ret'}}
// expected-note@+1{{recursive function located here:}}
float test_ret()
{
    return test_ret();
}

float4 main(float a : A, float b:B) : SV_TARGET
{
  return test_ret();
}

