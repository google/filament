@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn fwidthCoarse_159c8a() -> f32 {
  var res : f32 = fwidthCoarse(1.0f);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = fwidthCoarse_159c8a();
}
