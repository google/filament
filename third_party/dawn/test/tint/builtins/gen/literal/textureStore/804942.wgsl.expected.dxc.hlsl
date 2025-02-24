//
// fragment_main
//
RWTexture2D<int4> arg_0 : register(u0, space1);

void textureStore_804942() {
  arg_0[(1).xx] = (1).xxxx;
}

void fragment_main() {
  textureStore_804942();
  return;
}
//
// compute_main
//
RWTexture2D<int4> arg_0 : register(u0, space1);

void textureStore_804942() {
  arg_0[(1).xx] = (1).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_804942();
  return;
}
