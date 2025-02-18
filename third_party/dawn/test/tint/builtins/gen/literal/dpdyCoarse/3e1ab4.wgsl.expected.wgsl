@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn dpdyCoarse_3e1ab4() -> vec2<f32> {
  var res : vec2<f32> = dpdyCoarse(vec2<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = dpdyCoarse_3e1ab4();
}
