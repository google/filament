@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn saturate_270da5() -> f32 {
  var arg_0 = 2.0f;
  var res : f32 = saturate(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = saturate_270da5();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = saturate_270da5();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : f32,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = saturate_270da5();
  return out;
}
