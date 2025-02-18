
Texture2D<int4> t : register(t0, space1);
SamplerState s : register(s1, space1);
void main() {
  int4 res = t.GatherGreen(s, (0.0f).xx);
}

