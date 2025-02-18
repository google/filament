//
// fragment_main
//
RWTexture3D<int4> arg_0 : register(u0, space1);

void textureStore_a5c925() {
  arg_0[(1u).xxx] = (1).xxxx;
}

void fragment_main() {
  textureStore_a5c925();
  return;
}
//
// compute_main
//
RWTexture3D<int4> arg_0 : register(u0, space1);

void textureStore_a5c925() {
  arg_0[(1u).xxx] = (1).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_a5c925();
  return;
}
