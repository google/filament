@group(0) @binding(0) var<storage, read_write> prevent_dce : vec3<f32>;

fn dpdxCoarse_f64d7b() -> vec3<f32> {
  var res : vec3<f32> = dpdxCoarse(vec3<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = dpdxCoarse_f64d7b();
}
