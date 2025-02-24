@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn dpdyFine_d0a648() -> vec4<f32> {
  var res : vec4<f32> = dpdyFine(vec4<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = dpdyFine_d0a648();
}
