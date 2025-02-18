SKIP: INVALID


static vector<float16_t, 4> u = (float16_t(1.0h)).xxxx;
int4 tint_v4f16_to_v4i32(vector<float16_t, 4> value) {
  return (((value <= (float16_t(65504.0h)).xxxx)) ? ((((value >= (float16_t(-65504.0h)).xxxx)) ? (int4(value)) : ((int(-2147483648)).xxxx))) : ((int(2147483647)).xxxx));
}

void f() {
  int4 v = tint_v4f16_to_v4i32(u);
}

[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
<scrubbed_path>(2,15-23): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
