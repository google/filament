// Rewrite unchanged result:
const float4 planes[8];
[RootSignature("CBV(b0, space=0, visibility=SHADER_VISIBILITY_ALL)")]
float main() {
  float4 x = float4(1., 1., 1., 1.);
  for (uint i = 0; i < planes.Length; ++i) {
    x = x + planes[i];
  }
  return x.x;
}


