@group(0) @binding(0) var<storage, read_write> prevent_dce : vec2<f32>;

fn cosh_c13756() -> vec2<f32> {
  var res : vec2<f32> = cosh(vec2<f32>(0.0f));
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = cosh_c13756();
}

@compute @workgroup_size(1)
fn compute_main() {
  prevent_dce = cosh_c13756();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
  @location(0) @interpolate(flat)
  prevent_dce : vec2<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  out.prevent_dce = cosh_c13756();
  return out;
}
