enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<i32>;

fn subgroupMax_932164() -> vec2<i32> {
  var res : vec2<i32> = subgroupMax(vec2<i32>(1i));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMax_932164();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMax_932164();
}
