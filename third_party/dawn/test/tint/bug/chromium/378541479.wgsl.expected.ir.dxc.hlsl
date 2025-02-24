
cbuffer cbuffer_level : register(b0) {
  uint4 level[1];
};
cbuffer cbuffer_coords : register(b1) {
  uint4 coords[1];
};
Texture2D tex : register(t2);
[numthreads(1, 1, 1)]
void compute_main() {
  uint v = level[0u].x;
  int2 v_1 = int2(coords[0u].xy);
  float res = tex.Load(int3(v_1, int(v))).x;
}

