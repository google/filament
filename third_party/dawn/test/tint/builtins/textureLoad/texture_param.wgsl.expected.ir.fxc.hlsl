//
// vertex_main
//
struct vertex_main_outputs {
  float4 tint_symbol : SV_Position;
};


Texture2D<int4> arg_0 : register(t0, space1);
int4 textureLoad2d(Texture2D<int4> v, int2 coords, int level) {
  return v.Load(int3(coords, level));
}

void doTextureLoad() {
  int4 res = textureLoad2d(arg_0, (int(0)).xx, int(0));
}

float4 vertex_main_inner() {
  doTextureLoad();
  return (0.0f).xxxx;
}

vertex_main_outputs vertex_main() {
  vertex_main_outputs v_1 = {vertex_main_inner()};
  return v_1;
}

//
// fragment_main
//

Texture2D<int4> arg_0 : register(t0, space1);
int4 textureLoad2d(Texture2D<int4> v, int2 coords, int level) {
  return v.Load(int3(coords, level));
}

void doTextureLoad() {
  int4 res = textureLoad2d(arg_0, (int(0)).xx, int(0));
}

void fragment_main() {
  doTextureLoad();
}

//
// compute_main
//

Texture2D<int4> arg_0 : register(t0, space1);
int4 textureLoad2d(Texture2D<int4> v, int2 coords, int level) {
  return v.Load(int3(coords, level));
}

void doTextureLoad() {
  int4 res = textureLoad2d(arg_0, (int(0)).xx, int(0));
}

[numthreads(1, 1, 1)]
void compute_main() {
  doTextureLoad();
}

