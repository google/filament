@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

struct SB_RW {
  arg_0 : atomic<u32>,
}

@group(0) @binding(1) var<storage, read_write> sb_rw : SB_RW;

fn atomicAnd_85a8d9() -> u32 {
  var arg_1 = 1u;
  var res : u32 = atomicAnd(&(sb_rw.arg_0), arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = atomicAnd_85a8d9();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = atomicAnd_85a8d9();
}
