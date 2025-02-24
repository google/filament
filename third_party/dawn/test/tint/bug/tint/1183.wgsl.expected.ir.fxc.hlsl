struct f_outputs {
  float4 tint_symbol : SV_Target0;
};


Texture2D<float4> t : register(t0);
SamplerState s : register(s0, space1);
float4 f_inner() {
  return t.Sample(s, (0.0f).xx, int2(int(4), int(6)));
}

f_outputs f() {
  f_outputs v = {f_inner()};
  return v;
}

