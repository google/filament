//
// fragment_main
//
RWTexture1D<int4> arg_0 : register(u0, space1);

void textureStore_5a2f8f() {
  arg_0[1] = (1).xxxx;
}

void fragment_main() {
  textureStore_5a2f8f();
  return;
}
//
// compute_main
//
RWTexture1D<int4> arg_0 : register(u0, space1);

void textureStore_5a2f8f() {
  arg_0[1] = (1).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_5a2f8f();
  return;
}
