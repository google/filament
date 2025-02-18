fn cosh_c892bb() {
  var res = cosh(0.0);
}

@fragment
fn fragment_main() {
  cosh_c892bb();
}

@compute @workgroup_size(1)
fn compute_main() {
  cosh_c892bb();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  cosh_c892bb();
  return out;
}
