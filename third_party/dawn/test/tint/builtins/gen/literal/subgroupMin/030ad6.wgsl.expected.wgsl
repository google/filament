enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn subgroupMin_030ad6() -> vec3<i32> {
  var res : vec3<i32> = subgroupMin(vec3<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMin_030ad6();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMin_030ad6();
}
