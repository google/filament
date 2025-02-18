fn frexp_4b2200() {
  var res = frexp(1.0f);
}

@fragment
fn fragment_main() {
  frexp_4b2200();
}

@compute @workgroup_size(1)
fn compute_main() {
  frexp_4b2200();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  frexp_4b2200();
  return out;
}
