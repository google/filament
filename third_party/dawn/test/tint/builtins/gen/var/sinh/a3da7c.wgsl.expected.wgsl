fn sinh_a3da7c() {
  const arg_0 = vec4(1.0);
  var res = sinh(arg_0);
}

@fragment
fn fragment_main() {
  sinh_a3da7c();
}

@compute @workgroup_size(1)
fn compute_main() {
  sinh_a3da7c();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  sinh_a3da7c();
  return out;
}
