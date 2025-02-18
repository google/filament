fn ldexp_376938() {
  var res = ldexp(vec4(1.0), vec4<i32>(1i));
}

@fragment
fn fragment_main() {
  ldexp_376938();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_376938();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  ldexp_376938();
  return out;
}
