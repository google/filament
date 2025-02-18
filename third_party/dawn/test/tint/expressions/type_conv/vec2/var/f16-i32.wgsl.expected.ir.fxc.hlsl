SKIP: INVALID


static vector<float16_t, 2> u = (float16_t(1.0h)).xx;
int2 tint_v2f16_to_v2i32(vector<float16_t, 2> value) {
  return (((value <= (float16_t(65504.0h)).xx)) ? ((((value >= (float16_t(-65504.0h)).xx)) ? (int2(value)) : ((int(-2147483648)).xx))) : ((int(2147483647)).xx));
}

void f() {
  int2 v = tint_v2f16_to_v2i32(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
<scrubbed_path>(2,15-23): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
