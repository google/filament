enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn subgroupMul_93eccd() -> vec3<f32> {
  var arg_0 = vec3<f32>(1.0f);
  var res : vec3<f32> = subgroupMul(arg_0);
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
