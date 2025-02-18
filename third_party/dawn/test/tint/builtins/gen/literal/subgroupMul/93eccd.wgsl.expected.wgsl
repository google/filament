enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn subgroupMul_93eccd() -> vec3<f32> {
  var res : vec3<f32> = subgroupMul(vec3<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMul_93eccd();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMul_93eccd();
}
