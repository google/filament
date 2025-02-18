enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn subgroupOr_0bc264() -> u32 {
  var arg_0 = 1u;
  var res : u32 = subgroupOr(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupOr_0bc264();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupOr_0bc264();
}
