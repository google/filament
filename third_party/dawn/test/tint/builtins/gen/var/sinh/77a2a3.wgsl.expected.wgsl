fn sinh_77a2a3() {
  const arg_0 = vec3(1.0);
  var res = sinh(arg_0);
}

@fragment
fn fragment_main() {
  sinh_77a2a3();
}

@compute @workgroup_size(1)
fn compute_main() {
  sinh_77a2a3();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  sinh_77a2a3();
  return out;
}
