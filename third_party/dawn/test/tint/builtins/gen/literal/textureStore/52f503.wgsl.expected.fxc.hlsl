//
// fragment_main
//
RWTexture2D<int4> arg_0 : register(u0, space1);

void textureStore_52f503() {
  arg_0[(1u).xx] = (1).xxxx;
}

void fragment_main() {
  textureStore_52f503();
  return;
}
//
// compute_main
//
RWTexture2D<int4> arg_0 : register(u0, space1);

void textureStore_52f503() {
  arg_0[(1u).xx] = (1).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_52f503();
  return;
}
