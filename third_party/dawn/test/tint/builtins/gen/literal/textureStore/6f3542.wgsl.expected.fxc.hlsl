//
// fragment_main
//
RWTexture3D<uint4> arg_0 : register(u0, space1);

void textureStore_6f3542() {
  arg_0[(1).xxx] = (1u).xxxx;
}

void fragment_main() {
  textureStore_6f3542();
  return;
}
//
// compute_main
//
RWTexture3D<uint4> arg_0 : register(u0, space1);

void textureStore_6f3542() {
  arg_0[(1).xxx] = (1u).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_6f3542();
  return;
}
