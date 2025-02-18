SKIP: INVALID

[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

static float3x4 u = float3x4(float4(1.0f, 2.0f, 3.0f, 4.0f), float4(5.0f, 6.0f, 7.0f, 8.0f), float4(9.0f, 10.0f, 11.0f, 12.0f));

void f() {
  matrix<float16_t, 3, 4> v = matrix<float16_t, 3, 4>(u);
}
FXC validation failure:
<scrubbed_path>(9,10-18): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
