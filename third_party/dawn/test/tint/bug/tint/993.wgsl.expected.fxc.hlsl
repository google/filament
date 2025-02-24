cbuffer cbuffer_constants : register(b0, space1) {
  uint4 constants[1];
};

RWByteAddressBuffer result : register(u1, space1);

RWByteAddressBuffer s : register(u0);

int satomicLoad(uint offset) {
  int value = 0;
  s.InterlockedOr(offset, 0, value);
  return value;
}


int runTest() {
  return satomicLoad((4u * min((0u + uint(constants[0].x)), 2u)));
}

[numthreads(1, 1, 1)]
void main() {
  int tint_symbol = runTest();
  result.Store(0u, asuint(uint(tint_symbol)));
  return;
}
