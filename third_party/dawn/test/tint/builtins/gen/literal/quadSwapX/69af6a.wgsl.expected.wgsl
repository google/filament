enable subgroups;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<f32>;

fn quadSwapX_69af6a() -> vec4<f32> {
  var res : vec4<f32> = quadSwapX(vec4<f32>(1.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = quadSwapX_69af6a();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = quadSwapX_69af6a();
}
