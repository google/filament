fn clamp_177548() {
  var res = clamp(vec2(1), vec2(1), vec2(1));
}

@fragment
fn fragment_main() {
  clamp_177548();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_177548();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  clamp_177548();
  return out;
}
