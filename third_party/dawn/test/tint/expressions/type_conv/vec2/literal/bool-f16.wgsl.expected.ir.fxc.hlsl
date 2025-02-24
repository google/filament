SKIP: INVALID


static vector<float16_t, 2> u = (float16_t(1.0h)).xx;
[numthreads(1, 1, 1)]
void unused_entry_point() {
}

FXC validation failure:
<scrubbed_path>(2,15-23): error X3000: syntax error: unexpected token 'float16_t'


tint executable returned error: exit status 1
