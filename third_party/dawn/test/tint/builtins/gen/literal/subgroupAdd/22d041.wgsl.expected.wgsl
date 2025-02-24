enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn subgroupAdd_22d041() -> vec3<i32> {
  var res : vec3<i32> = subgroupAdd(vec3<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupAdd_22d041();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAdd_22d041();
}
