
static vector<float16_t, 3> u = (float16_t(1.0h)).xxx;
int3 tint_v3f16_to_v3i32(vector<float16_t, 3> value) {
  return (((value <= (float16_t(65504.0h)).xxx)) ? ((((value >= (float16_t(-65504.0h)).xxx)) ? (int3(value)) : ((int(-2147483648)).xxx))) : ((int(2147483647)).xxx));
}

void f() {
  int3 v = tint_v3f16_to_v3i32(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

