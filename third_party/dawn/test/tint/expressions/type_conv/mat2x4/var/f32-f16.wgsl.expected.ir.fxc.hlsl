SKIP: INVALID


static float2x4 u = float2x4(float4(1.0f, 2.0f, 3.0f, 4.0f), float4(5.0f, 6.0f, 7.0f, 8.0f));
void f() {
  matrix<float16_t, 2, 4> v = matrix<float16_t, 2, 4>(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
<scrubbed_path>(4,10-18): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
