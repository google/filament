fn cosh_432645() {
  var res = cosh(vec2(0.0));
}

@fragment
fn fragment_main() {
  cosh_432645();
}

@compute @workgroup_size(1)
fn compute_main() {
  cosh_432645();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  cosh_432645();
  return out;
}
