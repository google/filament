Texture2D tex : register(t0);
RWByteAddressBuffer result : register(u1);

struct tint_symbol_1 {
  uint3 GlobalInvocationId : SV_DispatchThreadID;
};

void main_inner(uint3 GlobalInvocationId) {
  uint tint_symbol_3 = 0u;
  result.GetDimensions(tint_symbol_3);
  uint tint_symbol_4 = ((tint_symbol_3 - 0u) / 4u);
  result.Store((4u * min(((GlobalInvocationId.y * 128u) + GlobalInvocationId.x), (tint_symbol_4 - 1u))), asuint(tex.Load(int3(int(GlobalInvocationId.x), int(GlobalInvocationId.y), 0)).x));
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.GlobalInvocationId);
  return;
}
