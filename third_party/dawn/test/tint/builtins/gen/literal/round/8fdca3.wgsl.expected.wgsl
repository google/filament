fn round_8fdca3() {
  var res = round(vec2(3.5));
}

@fragment
fn fragment_main() {
  round_8fdca3();
}

@compute @workgroup_size(1)
fn compute_main() {
  round_8fdca3();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  round_8fdca3();
  return out;
}
