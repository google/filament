struct SB_RW_atomic {
  /* @offset(0) */
  arg_0 : atomic<u32>,
}

struct SB_RW {
  /* @offset(0) */
  arg_0 : u32,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW_atomic;

fn atomicOr_5e95d4() {
  var arg_1 = 0u;
  var res = 0u;
  arg_1 = 1u;
  let x_18 = arg_1;
  let x_13 = atomicOr(&(sb_rw.arg_0), x_18);
  res = x_13;
  return;
}

fn fragment_main_1() {
  atomicOr_5e95d4();
  return;
}

@fragment
fn fragment_main() {
  fragment_main_1();
}

fn compute_main_1() {
  atomicOr_5e95d4();
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn compute_main() {
  compute_main_1();
}
