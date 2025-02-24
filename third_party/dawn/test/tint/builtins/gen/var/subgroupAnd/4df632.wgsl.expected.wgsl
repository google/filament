enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn subgroupAnd_4df632() -> u32 {
  var arg_0 = 1u;
  var res : u32 = subgroupAnd(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupAnd_4df632();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAnd_4df632();
}
