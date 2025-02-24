fn round_a1673d() {
  var res = round(vec3(3.5));
}

@fragment
fn fragment_main() {
  round_a1673d();
}

@compute @workgroup_size(1)
fn compute_main() {
  round_a1673d();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  round_a1673d();
  return out;
}
