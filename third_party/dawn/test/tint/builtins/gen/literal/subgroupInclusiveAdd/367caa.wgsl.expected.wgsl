enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn subgroupInclusiveAdd_367caa() -> vec4<f32> {
  var res : vec4<f32> = subgroupInclusiveAdd(vec4<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupInclusiveAdd_367caa();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupInclusiveAdd_367caa();
}
