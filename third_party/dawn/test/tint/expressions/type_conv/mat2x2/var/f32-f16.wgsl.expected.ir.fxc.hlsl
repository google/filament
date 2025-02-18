SKIP: INVALID


static float2x2 u = float2x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f));
void f() {
  matrix<float16_t, 2, 2> v = matrix<float16_t, 2, 2>(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
<scrubbed_path>(4,10-18): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
