enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn subgroupAdd_1eb429() -> vec2<i32> {
  var arg_0 = vec2<i32>(1i);
  var res : vec2<i32> = subgroupAdd(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupAdd_1eb429();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupAdd_1eb429();
}
