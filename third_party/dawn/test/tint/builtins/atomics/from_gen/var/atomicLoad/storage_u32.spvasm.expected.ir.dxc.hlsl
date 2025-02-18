//
// fragment_main
//

RWByteAddressBuffer sb_rw : register(u0);
void atomicLoad_fe6cc3() {
  uint res = 0u;
  uint v = 0u;
  sb_rw.InterlockedOr(0u, 0u, v);
  uint x_9 = v;
  res = x_9;
}

void fragment_main_1() {
  atomicLoad_fe6cc3();
}

void fragment_main() {
  fragment_main_1();
}

//
// compute_main
//

RWByteAddressBuffer sb_rw : register(u0);
void atomicLoad_fe6cc3() {
  uint res = 0u;
  uint v = 0u;
  sb_rw.InterlockedOr(0u, 0u, v);
  uint x_9 = v;
  res = x_9;
}

void compute_main_1() {
  atomicLoad_fe6cc3();
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
}

