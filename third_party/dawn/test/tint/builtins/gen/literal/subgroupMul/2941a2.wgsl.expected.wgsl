enable subgroups;
enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn subgroupMul_2941a2() -> f16 {
  var res : f16 = subgroupMul(1.0h);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMul_2941a2();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMul_2941a2();
}
