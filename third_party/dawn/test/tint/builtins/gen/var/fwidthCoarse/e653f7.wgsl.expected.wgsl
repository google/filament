@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn fwidthCoarse_e653f7() -> vec2<f32> {
  var arg_0 = vec2<f32>(1.0f);
  var res : vec2<f32> = fwidthCoarse(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = fwidthCoarse_e653f7();
}
