//
// fragment_main
//
RWTexture3D<int4> arg_0 : register(u0, space1);

void textureStore_07548b() {
  arg_0[(1u).xxx] = (1).xxxx;
}

void fragment_main() {
  textureStore_07548b();
  return;
}
//
// compute_main
//
RWTexture3D<int4> arg_0 : register(u0, space1);

void textureStore_07548b() {
  arg_0[(1u).xxx] = (1).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_07548b();
  return;
}
