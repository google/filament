fn cosh_f67ff1() {
  const arg_0 = vec3(0.0);
  var res = cosh(arg_0);
}

@fragment
fn fragment_main() {
  cosh_f67ff1();
}

@compute @workgroup_size(1)
fn compute_main() {
  cosh_f67ff1();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  cosh_f67ff1();
  return out;
}
