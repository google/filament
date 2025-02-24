struct SB_RW_atomic {
  /* @offset(0) */
  arg_0 : atomic<i32>,
}

struct SB_RW {
  /* @offset(0) */
  arg_0 : i32,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW_atomic;

fn atomicMin_8e38dc() {
  var arg_1 = 0i;
  var res = 0i;
  arg_1 = 1i;
  let x_20 = arg_1;
  let x_13 = atomicMin(&(sb_rw.arg_0), x_20);
  res = x_13;
  return;
}

fn fragment_main_1() {
  atomicMin_8e38dc();
  return;
}

@fragment
fn fragment_main() {
  fragment_main_1();
}

fn compute_main_1() {
  atomicMin_8e38dc();
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn compute_main() {
  compute_main_1();
}
