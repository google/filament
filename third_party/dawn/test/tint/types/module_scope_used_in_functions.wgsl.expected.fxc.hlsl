groupshared float w;

void tint_zero_workgroup_memory(uint local_idx) {
  if ((local_idx < 1u)) {
    w = 0.0f;
  }
  GroupMemoryBarrierWithGroupSync();
}

static float p = 0.0f;
ByteAddressBuffer uniforms : register(t1);
RWByteAddressBuffer storages : register(u0);

void no_uses() {
}

void zoo() {
  p = (p * 2.0f);
}

void bar(float a, float b) {
  uint tint_symbol_3 = 0u;
  storages.GetDimensions(tint_symbol_3);
  uint tint_symbol_4 = (tint_symbol_3 / 4u);
  p = a;
  w = b;
  storages.Store((4u * min(0u, (tint_symbol_4 - 1u))), asuint(asfloat(uniforms.Load(0u))));
  zoo();
}

void foo(float a) {
  float b = 2.0f;
  bar(a, b);
  no_uses();
}

struct tint_symbol_1 {
  uint local_invocation_index : SV_GroupIndex;
};

void main_inner(uint local_invocation_index) {
  tint_zero_workgroup_memory(local_invocation_index);
  foo(1.0f);
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.local_invocation_index);
  return;
}
