fn frexp_77af93() {
  var res = frexp(vec4<f32>(1.0f));
}

@fragment
fn fragment_main() {
  frexp_77af93();
}

@compute @workgroup_size(1)
fn compute_main() {
  frexp_77af93();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  frexp_77af93();
  return out;
}
