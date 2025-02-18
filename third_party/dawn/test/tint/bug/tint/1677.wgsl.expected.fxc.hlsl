ByteAddressBuffer input : register(t0);

struct tint_symbol_1 {
  uint3 id : SV_DispatchThreadID;
};

void main_inner(uint3 id) {
  int3 pos = (asint(input.Load3(0u)) - (0).xxx);
}

[numthreads(1, 1, 1)]
void main(tint_symbol_1 tint_symbol) {
  main_inner(tint_symbol.id);
  return;
}
