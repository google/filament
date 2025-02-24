@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn dpdxFine_8c5069() -> vec4<f32> {
  var res : vec4<f32> = dpdxFine(vec4<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = dpdxFine_8c5069();
}
