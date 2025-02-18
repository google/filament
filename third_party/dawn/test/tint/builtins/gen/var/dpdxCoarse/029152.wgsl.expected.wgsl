@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn dpdxCoarse_029152() -> f32 {
  var arg_0 = 1.0f;
  var res : f32 = dpdxCoarse(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = dpdxCoarse_029152();
}
