fn frexp_bf45ae() {
  const arg_0 = vec3(1.0);
  var res = frexp(arg_0);
}

@fragment
fn fragment_main() {
  frexp_bf45ae();
}

@compute @workgroup_size(1)
fn compute_main() {
  frexp_bf45ae();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  frexp_bf45ae();
  return out;
}
