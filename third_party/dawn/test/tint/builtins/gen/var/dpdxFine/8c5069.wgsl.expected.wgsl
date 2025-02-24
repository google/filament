@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn dpdxFine_8c5069() -> vec4<f32> {
  var arg_0 = vec4<f32>(1.0f);
  var res : vec4<f32> = dpdxFine(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = dpdxFine_8c5069();
}
