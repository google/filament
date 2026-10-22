

// Make sure same cbuffer decalred in different lib works.

cbuffer A {
  float a;
  float v;
}

Texture2D	tex;
SamplerState	samp;

export float GetV() {
  return v + tex.Load(uint3(a, v, a)).y;
}
