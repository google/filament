fn ldexp_2bfc68() {
  var res = ldexp(vec2(1.0), vec2<i32>(1i));
}

@fragment
fn fragment_main() {
  ldexp_2bfc68();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_2bfc68();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  ldexp_2bfc68();
  return out;
}
