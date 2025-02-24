
static vector<float16_t, 4> u = (float16_t(1.0h)).xxxx;
void f() {
  float4 v = float4(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

