//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

RWByteAddressBuffer sb_rw : register(u1);

uint sb_rwatomicXor(uint offset, uint value) {
  uint original_value = 0;
  sb_rw.InterlockedXor(offset, value, original_value);
  return original_value;
}


uint atomicXor_54510e() {
  uint arg_1 = 1u;
  uint res = sb_rwatomicXor(0u, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(atomicXor_54510e()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

RWByteAddressBuffer sb_rw : register(u1);

uint sb_rwatomicXor(uint offset, uint value) {
  uint original_value = 0;
  sb_rw.InterlockedXor(offset, value, original_value);
  return original_value;
}


uint atomicXor_54510e() {
  uint arg_1 = 1u;
  uint res = sb_rwatomicXor(0u, arg_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(atomicXor_54510e()));
  return;
}
