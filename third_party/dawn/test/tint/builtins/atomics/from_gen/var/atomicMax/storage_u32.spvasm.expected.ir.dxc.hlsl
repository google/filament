//
// fragment_main
//

RWByteAddressBuffer sb_rw : register(u0);
void atomicMax_51b9be() {
  uint arg_1 = 0u;
  uint res = 0u;
  arg_1 = 1u;
  uint x_18 = arg_1;
  uint v = 0u;
  sb_rw.InterlockedMax(0u, x_18, v);
  uint x_13 = v;
  res = x_13;
}

void fragment_main_1() {
  atomicMax_51b9be();
}

void fragment_main() {
  fragment_main_1();
}

//
// compute_main
//

RWByteAddressBuffer sb_rw : register(u0);
void atomicMax_51b9be() {
  uint arg_1 = 0u;
  uint res = 0u;
  arg_1 = 1u;
  uint x_18 = arg_1;
  uint v = 0u;
  sb_rw.InterlockedMax(0u, x_18, v);
  uint x_13 = v;
  res = x_13;
}

void compute_main_1() {
  atomicMax_51b9be();
}

[numthreads(1, 1, 1)]
void compute_main() {
  compute_main_1();
}

