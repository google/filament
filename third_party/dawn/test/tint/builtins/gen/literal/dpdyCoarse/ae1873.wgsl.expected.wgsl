@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn dpdyCoarse_ae1873() -> vec3<f32> {
  var res : vec3<f32> = dpdyCoarse(vec3<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = dpdyCoarse_ae1873();
}
