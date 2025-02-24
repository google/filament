SKIP: INVALID


static matrix<float16_t, 4, 4> u = matrix<float16_t, 4, 4>(vector<float16_t, 4>(float16_t(1.0h), float16_t(2.0h), float16_t(3.0h), float16_t(4.0h)), vector<float16_t, 4>(float16_t(5.0h), float16_t(6.0h), float16_t(7.0h), float16_t(8.0h)), vector<float16_t, 4>(float16_t(9.0h), float16_t(10.0h), float16_t(11.0h), float16_t(12.0h)), vector<float16_t, 4>(float16_t(13.0h), float16_t(14.0h), float16_t(15.0h), float16_t(16.0h)));
void f() {
  float4x4 v = float4x4(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
<scrubbed_path>(2,15-23): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
