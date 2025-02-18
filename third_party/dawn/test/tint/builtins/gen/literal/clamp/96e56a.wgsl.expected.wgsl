fn clamp_96e56a() {
  var res = clamp(1, 1, 1);
}

@fragment
fn fragment_main() {
  clamp_96e56a();
}

@compute @workgroup_size(1)
fn compute_main() {
  clamp_96e56a();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  clamp_96e56a();
  return out;
}
