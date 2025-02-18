enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn subgroupMin_0bc13a() -> vec2<i32> {
  var res : vec2<i32> = subgroupMin(vec2<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMin_0bc13a();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMin_0bc13a();
}
