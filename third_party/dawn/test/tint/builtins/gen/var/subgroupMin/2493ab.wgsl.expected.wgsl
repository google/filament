enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : u32;

fn subgroupMin_2493ab() -> u32 {
  var arg_0 = 1u;
  var res : u32 = subgroupMin(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMin_2493ab();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMin_2493ab();
}
