fn ldexp_cb0faf() {
  var res = ldexp(vec4(1.0), vec4(1));
}

@fragment
fn fragment_main() {
  ldexp_cb0faf();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_cb0faf();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  ldexp_cb0faf();
  return out;
}
