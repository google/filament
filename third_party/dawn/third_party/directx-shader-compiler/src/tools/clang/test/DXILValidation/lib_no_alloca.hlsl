// Make sure no alloca for linked dxil module.

float2 test(float2 a);

[shader("pixel")]
float4 ps_main(float2 a : A) : SV_TARGET
{
  return test(a).y;
}

