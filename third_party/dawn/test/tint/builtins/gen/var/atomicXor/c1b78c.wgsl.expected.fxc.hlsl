//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

RWByteAddressBuffer sb_rw : register(u1);

int sb_rwatomicXor(uint offset, int value) {
  int original_value = 0;
  sb_rw.InterlockedXor(offset, value, original_value);
  return original_value;
}


int atomicXor_c1b78c() {
  int arg_1 = 1;
  int res = sb_rwatomicXor(0u, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(atomicXor_c1b78c()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

RWByteAddressBuffer sb_rw : register(u1);

int sb_rwatomicXor(uint offset, int value) {
  int original_value = 0;
  sb_rw.InterlockedXor(offset, value, original_value);
  return original_value;
}


int atomicXor_c1b78c() {
  int arg_1 = 1;
  int res = sb_rwatomicXor(0u, arg_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(atomicXor_c1b78c()));
  return;
}
