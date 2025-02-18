enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn subgroupAnd_1877b3() -> vec3<i32> {
  var arg_0 = vec3<i32>(1i);
  var res : vec3<i32> = subgroupAnd(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupAnd_1877b3();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAnd_1877b3();
}
