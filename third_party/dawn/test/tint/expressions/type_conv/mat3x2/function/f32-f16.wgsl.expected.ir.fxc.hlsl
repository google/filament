SKIP: INVALID


static float t = 0.0f;
float3x2 m() {
  t = (t + 1.0f);
  return float3x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f), float2(5.0f, 6.0f));
}

void f() {
  matrix<float16_t, 3, 2> v = matrix<float16_t, 3, 2>(m());
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
<scrubbed_path>(9,10-18): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
