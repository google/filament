//
// fragment_main
//

RWTexture2DArray<uint4> arg_0 : register(u0, space1);
void textureStore_00ca64() {
  arg_0[int3((int(1)).xx, int(1u))] = (1u).xxxx;
}

void fragment_main() {
  textureStore_00ca64();
}

//
// compute_main
//

RWTexture2DArray<uint4> arg_0 : register(u0, space1);
void textureStore_00ca64() {
  arg_0[int3((int(1)).xx, int(1u))] = (1u).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_00ca64();
}

