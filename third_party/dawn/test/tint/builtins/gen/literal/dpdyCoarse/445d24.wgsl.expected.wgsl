@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn dpdyCoarse_445d24() -> vec4<f32> {
  var res : vec4<f32> = dpdyCoarse(vec4<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = dpdyCoarse_445d24();
}
