enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn subgroupMin_2d8828() -> vec2<f32> {
  var res : vec2<f32> = subgroupMin(vec2<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = subgroupMin_2d8828();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = subgroupMin_2d8828();
}
