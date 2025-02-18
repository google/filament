struct SB_RW_atomic {
  /* @offset(0) */
  arg_0 : atomic<u32>,
}

struct SB_RW {
  /* @offset(0) */
  arg_0 : u32,
}

@group(0) @binding(0) var<storage, read_write> sb_rw : SB_RW_atomic;

fn atomicMax_51b9be() {
  var res = 0u;
  let x_9 = atomicMax(&(sb_rw.arg_0), 1u);
  res = x_9;
  return;
}

fn fragment_main_1() {
  atomicMax_51b9be();
  return;
}

@fragment
fn fragment_main() {
  fragment_main_1();
}

fn compute_main_1() {
  atomicMax_51b9be();
  return;
}

@compute @workgroup_size(1i, 1i, 1i)
fn compute_main() {
  compute_main_1();
}
