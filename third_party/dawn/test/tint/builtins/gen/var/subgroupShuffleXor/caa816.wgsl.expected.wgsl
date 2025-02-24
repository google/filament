enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn subgroupShuffleXor_caa816() -> vec3<f32> {
  var arg_0 = vec3<f32>(1.0f);
  var arg_1 = 1u;
  var res : vec3<f32> = subgroupShuffleXor(arg_0, arg_1);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupShuffleXor_caa816();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupShuffleXor_caa816();
}
