struct main_inputs {
  uint3 id : SV_DispatchThreadID;
};


ByteAddressBuffer input : register(t0);
void main_inner(uint3 id) {
  int3 pos = (asint(input.Load3(0u)) - (int(0)).xxx);
}

[numthreads(1, 1, 1)]
void main(main_inputs inputs) {
  main_inner(inputs.id);
}

