fn ldexp_fdbc7b() {
  var res = ldexp(1.0, 1);
}

@fragment
fn fragment_main() {
  ldexp_fdbc7b();
}

@compute @workgroup_size(1)
fn compute_main() {
  ldexp_fdbc7b();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  ldexp_fdbc7b();
  return out;
}
