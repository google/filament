//
// fragment_main
//
RWTexture2D<int4> arg_0 : register(u0, space1);

void textureStore_9f5318() {
  arg_0[(1u).xx] = (1).xxxx;
}

void fragment_main() {
  textureStore_9f5318();
  return;
}
//
// compute_main
//
RWTexture2D<int4> arg_0 : register(u0, space1);

void textureStore_9f5318() {
  arg_0[(1u).xx] = (1).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_9f5318();
  return;
}
