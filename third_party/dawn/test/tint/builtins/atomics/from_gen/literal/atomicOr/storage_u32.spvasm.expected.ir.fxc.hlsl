//
// fragment_main
//

RWByteAddressBuffer sb_rw : register(u0);
void atomicOr_5e95d4() {
  uint res = 0u;
  uint v = 0u;
  sb_rw.InterlockedOr(0u, 1u, v);
  uint x_9 = v;
  res = x_9;
}

void fragment_main_1() {
  atomicOr_5e95d4();
}

void fragment_main() {
  fragment_main_1();
}

//
// compute_main
//

RWByteAddressBuffer sb_rw : register(u0);
void atomicOr_5e95d4() {
  uint res = 0u;
  uint v = 0u;
  sb_rw.InterlockedOr(0u, 1u, v);
  uint x_9 = v;
  res = x_9;
}

void compute_main_1() {
  atomicOr_5e95d4();
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
}

