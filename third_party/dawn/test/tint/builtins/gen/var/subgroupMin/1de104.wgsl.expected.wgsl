enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<i32>;

fn subgroupMin_1de104() -> vec4<i32> {
  var arg_0 = vec4<i32>(1i);
  var res : vec4<i32> = subgroupMin(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMin_1de104();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMin_1de104();
}
