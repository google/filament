fn frexp_34bbfb() {
  const arg_0 = vec4(1.0);
  var res = frexp(arg_0);
}

@fragment
fn fragment_main() {
  frexp_34bbfb();
}

@compute @workgroup_size(1)
fn compute_main() {
  frexp_34bbfb();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  frexp_34bbfb();
  return out;
}
