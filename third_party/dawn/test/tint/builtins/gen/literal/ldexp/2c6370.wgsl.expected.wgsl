fn ldexp_2c6370() {
  var res = ldexp(vec2(1.0), vec2(1));
}

@fragment
fn fragment_main() {
  ldexp_2c6370();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_2c6370();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  ldexp_2c6370();
  return out;
}
