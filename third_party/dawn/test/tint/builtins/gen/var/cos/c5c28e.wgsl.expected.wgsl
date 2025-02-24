@group(0) @binding(0) var<storage, read_write> prevent_dce : f32;

fn cos_c5c28e() -> f32 {
  var arg_0 = 0.0f;
  var res : f32 = cos(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = cos_c5c28e();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = cos_c5c28e();
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
  out.prevent_dce = cos_c5c28e();
  return out;
}
