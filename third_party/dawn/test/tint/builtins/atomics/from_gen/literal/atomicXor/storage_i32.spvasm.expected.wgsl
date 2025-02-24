struct SB_RW_atomic {
  /* @offset(0) */
  arg_0 : atomic<i32>,
}

struct SB_RW {
  /* @offset(0) */
  arg_0 : i32,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW_atomic;

fn atomicXor_c1b78c() {
  var res = 0i;
  let x_9 = atomicXor(&(sb_rw.arg_0), 1i);
  res = x_9;
  return;
}

fn fragment_main_1() {
  atomicXor_c1b78c();
  return;
}

@fragment
fn fragment_main() {
  fragment_main_1();
}

fn compute_main_1() {
  atomicXor_c1b78c();
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn compute_main() {
  compute_main_1();
}
