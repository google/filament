Texture2D<float4> t : register(t0, space1);
Texture2D d : register(t1, space1);
SamplerState s : register(s0);
SamplerComparisonState sc : register(s1);

void main() {
  float4 a = t.Sample(s, (1.0f).xx);
  float4 b = d.GatherCmp(sc, (1.0f).xx, 1.0f);
  return;
}
