@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn dpdxCoarse_c28641() -> vec4<f32> {
  var arg_0 = vec4<f32>(1.0f);
  var res : vec4<f32> = dpdxCoarse(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = dpdxCoarse_c28641();
}
