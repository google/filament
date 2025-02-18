enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn subgroupExclusiveAdd_41cfde() -> vec3<f32> {
  var res : vec3<f32> = subgroupExclusiveAdd(vec3<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupExclusiveAdd_41cfde();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupExclusiveAdd_41cfde();
}
