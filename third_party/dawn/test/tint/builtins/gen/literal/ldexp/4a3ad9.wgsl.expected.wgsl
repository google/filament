fn ldexp_4a3ad9() {
  var res = ldexp(vec3(1.0), vec3(1));
}

@fragment
fn fragment_main() {
  ldexp_4a3ad9();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_4a3ad9();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  ldexp_4a3ad9();
  return out;
}
