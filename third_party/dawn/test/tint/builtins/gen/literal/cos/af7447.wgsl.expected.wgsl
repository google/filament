fn cos_af7447() {
  var res = cos(vec2(0.0));
}

@fragment
fn fragment_main() {
  cos_af7447();
}

@compute @workgroup_size(1)
fn compute_main() {
  cos_af7447();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  cos_af7447();
  return out;
}
