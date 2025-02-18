struct main_inputs {
  uint tint_local_index : SV_GroupIndex;
};


static float p = 0.0f;
groupshared float w;
ByteAddressBuffer uniforms : register(t1);
RWByteAddressBuffer storages : register(u0);
void no_uses() {
}

void zoo() {
  p = (p * 2.0f);
}

void bar(float a, float b) {
  p = a;
  w = b;
  uint v = 0u;
  storages.GetDimensions(v);
  uint v_1 = ((v / 4u) - 1u);
  uint v_2 = (min(uint(int(0)), v_1) * 4u);
  storages.Store((0u + v_2), asuint(asfloat(uniforms.Load(0u))));
  zoo();
}

void foo(float a) {
  float b = 2.0f;
  bar(a, b);
  no_uses();
}

void main_inner(uint tint_local_index) {
  if ((tint_local_index < 1u)) {
    w = 0.0f;
  }
  GroupMemoryBarrierWithGroupSync();
  foo(1.0f);
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.tint_local_index);
}

