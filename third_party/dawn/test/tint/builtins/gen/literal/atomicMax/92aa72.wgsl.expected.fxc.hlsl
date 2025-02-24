//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

RWByteAddressBuffer sb_rw : register(u1);

int sb_rwatomicMax(uint offset, int value) {
  int original_value = 0;
  sb_rw.InterlockedMax(offset, value, original_value);
  return original_value;
}


int atomicMax_92aa72() {
  int res = sb_rwatomicMax(0u, 1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(atomicMax_92aa72()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

RWByteAddressBuffer sb_rw : register(u1);

int sb_rwatomicMax(uint offset, int value) {
  int original_value = 0;
  sb_rw.InterlockedMax(offset, value, original_value);
  return original_value;
}


int atomicMax_92aa72() {
  int res = sb_rwatomicMax(0u, 1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(atomicMax_92aa72()));
  return;
}
