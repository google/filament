//
// fragment_main
//
RWTexture1D<int4> arg_0 : register(u0, space1);

void textureStore_6b80d2() {
  arg_0[1] = (1).xxxx;
}

void fragment_main() {
  textureStore_6b80d2();
  return;
}
//
// compute_main
//
RWTexture1D<int4> arg_0 : register(u0, space1);

void textureStore_6b80d2() {
  arg_0[1] = (1).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_6b80d2();
  return;
}
