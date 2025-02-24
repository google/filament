@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn dpdxFine_f92fb6() -> vec3<f32> {
  var res : vec3<f32> = dpdxFine(vec3<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = dpdxFine_f92fb6();
}
