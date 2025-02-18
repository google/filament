enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<i32>;

fn subgroupMax_4ea90e() -> vec3<i32> {
  var arg_0 = vec3<i32>(1i);
  var res : vec3<i32> = subgroupMax(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMax_4ea90e();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMax_4ea90e();
}
