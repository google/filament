enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn subgroupExclusiveMul_98b2e4() -> f32 {
  var arg_0 = 1.0f;
  var res : f32 = subgroupExclusiveMul(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupExclusiveMul_98b2e4();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupExclusiveMul_98b2e4();
}
