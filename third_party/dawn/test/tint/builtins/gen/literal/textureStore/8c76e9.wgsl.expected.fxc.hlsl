//
// fragment_main
//
RWTexture1D<int4> arg_0 : register(u0, space1);

void textureStore_8c76e9() {
  arg_0[1u] = (1).xxxx;
}

void fragment_main() {
  textureStore_8c76e9();
  return;
}
//
// compute_main
//
RWTexture1D<int4> arg_0 : register(u0, space1);

void textureStore_8c76e9() {
  arg_0[1u] = (1).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_8c76e9();
  return;
}
