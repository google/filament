//
// fragment_main
//
RWTexture1D<int4> arg_0 : register(u0, space1);

void textureStore_3d6f01() {
  arg_0[1u] = (1).xxxx;
}

void fragment_main() {
  textureStore_3d6f01();
  return;
}
//
// compute_main
//
RWTexture1D<int4> arg_0 : register(u0, space1);

void textureStore_3d6f01() {
  arg_0[1u] = (1).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_3d6f01();
  return;
}
