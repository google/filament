//
// fragment_main
//
RWByteAddressBuffer prevent_dce : register(u0);

RWByteAddressBuffer sb_rw : register(u1);

int sb_rwatomicExchange(uint offset, int value) {
  int original_value = 0;
  sb_rw.InterlockedExchange(offset, value, original_value);
  return original_value;
}


int atomicExchange_f2e22f() {
  int arg_1 = 1;
  int res = sb_rwatomicExchange(0u, arg_1);
  return res;
}

void fragment_main() {
  prevent_dce.Store(0u, asuint(atomicExchange_f2e22f()));
  return;
}
//
// compute_main
//
RWByteAddressBuffer prevent_dce : register(u0);

RWByteAddressBuffer sb_rw : register(u1);

int sb_rwatomicExchange(uint offset, int value) {
  int original_value = 0;
  sb_rw.InterlockedExchange(offset, value, original_value);
  return original_value;
}


int atomicExchange_f2e22f() {
  int arg_1 = 1;
  int res = sb_rwatomicExchange(0u, arg_1);
  return res;
}

[numthreads(1, 1, 1)]
void compute_main() {
  prevent_dce.Store(0u, asuint(atomicExchange_f2e22f()));
  return;
}
