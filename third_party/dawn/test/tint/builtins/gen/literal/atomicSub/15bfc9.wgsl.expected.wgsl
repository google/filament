@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

struct SB_RW {
  arg_0 : atomic<u32>,
}

@group(0) @binding(1) var<storage, read_write> sb_rw : SB_RW;

fn atomicSub_15bfc9() -> u32 {
  var res : u32 = atomicSub(&(sb_rw.arg_0), 1u);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = atomicSub_15bfc9();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = atomicSub_15bfc9();
}
