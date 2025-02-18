enable f16;

@group(0) @binding(0) var<storage, read_write> prevent_dce : f16;

fn cos_fc047d() -> f16 {
  var arg_0 = 0.0h;
  var res : f16 = cos(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = cos_fc047d();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = cos_fc047d();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : f16,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = cos_fc047d();
  return out;
}
