//
// fragment_main
//

RWTexture3D<uint4> arg_0 : register(u0, space1);
void textureStore_1efc36() {
  arg_0[(1u).xxx] = (1u).xxxx;
}

void fragment_main() {
  textureStore_1efc36();
}

//
// compute_main
//

RWTexture3D<uint4> arg_0 : register(u0, space1);
void textureStore_1efc36() {
  arg_0[(1u).xxx] = (1u).xxxx;
}

[numthreads(1, 1, 1)]
void compute_main() {
  textureStore_1efc36();
}

