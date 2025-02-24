// RUN: %dxc -Tlib_6_5 -verify %s 

// expected-error@+2{{recursive functions are not allowed: function 'recurse' calls recursive function 'recurse'}}
// expected-note@+1{{recursive function located here:}}
export void recurse(inout float4 f, float a) 
{
    if (a > 1)
      recurse(f, a-1);
    f = abs(f+a);
}

float4 main(float a : A, float b:B) : SV_TARGET
{
  float4 f = b;
  recurse(f, a);
  return f;
}
