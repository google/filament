
static int3 u = (int(1)).xxx;
void f() {
  vector<float16_t, 3> v = vector<float16_t, 3>(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

