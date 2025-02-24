
Texture2D<uint4> t : register(t0, space1);
SamplerState s : register(s1, space1);
void main() {
  uint4 res = t.GatherRed(s, (0.0f).xx);
}

