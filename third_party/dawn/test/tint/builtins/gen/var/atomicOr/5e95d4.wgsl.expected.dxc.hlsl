//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

RWByteAddressBuffer sb_rw : register(u1);

uint sb_rwatomicOr(uint offset, uint value) {
  uint original_value = 0;
  sb_rw.InterlockedOr(offset, value, original_value);
  return original_value;
}


uint atomicOr_5e95d4() {
  uint arg_1 = 1u;
  uint res = sb_rwatomicOr(0u, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(atomicOr_5e95d4()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

RWByteAddressBuffer sb_rw : register(u1);

uint sb_rwatomicOr(uint offset, uint value) {
  uint original_value = 0;
  sb_rw.InterlockedOr(offset, value, original_value);
  return original_value;
}


uint atomicOr_5e95d4() {
  uint arg_1 = 1u;
  uint res = sb_rwatomicOr(0u, arg_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(atomicOr_5e95d4()));
  return;
}
