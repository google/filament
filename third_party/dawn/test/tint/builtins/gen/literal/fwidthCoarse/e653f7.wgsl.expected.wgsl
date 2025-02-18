@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn fwidthCoarse_e653f7() -> vec2<f32> {
  var res : vec2<f32> = fwidthCoarse(vec2<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = fwidthCoarse_e653f7();
}
