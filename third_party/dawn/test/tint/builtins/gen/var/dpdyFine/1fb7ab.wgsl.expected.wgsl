@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn dpdyFine_1fb7ab() -> vec3<f32> {
  var arg_0 = vec3<f32>(1.0f);
  var res : vec3<f32> = dpdyFine(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = dpdyFine_1fb7ab();
}
