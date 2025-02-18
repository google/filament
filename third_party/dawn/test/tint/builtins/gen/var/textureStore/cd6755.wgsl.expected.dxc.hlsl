//
// fragment_main
//
RWTexture3D<uint4> arg_0 : register(u0, space1);

void textureStore_cd6755() {
  uint3 arg_1 = (1u).xxx;
  uint4 arg_2 = (1u).xxxx;
  arg_0[arg_1] = arg_2;
}

void fragment_main() {
  textureStore_cd6755();
  return;
}
//
// compute_main
//
RWTexture3D<uint4> arg_0 : register(u0, space1);

void textureStore_cd6755() {
  uint3 arg_1 = (1u).xxx;
  uint4 arg_2 = (1u).xxxx;
  arg_0[arg_1] = arg_2;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_cd6755();
  return;
}
