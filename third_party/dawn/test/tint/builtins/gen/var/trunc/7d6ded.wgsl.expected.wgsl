fn trunc_7d6ded() {
  const arg_0 = 1.5;
  var res = trunc(arg_0);
}

@fragment
fn fragment_main() {
  trunc_7d6ded();
}

@compute @workgroup_size(1)
fn compute_main() {
  trunc_7d6ded();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  trunc_7d6ded();
  return out;
}
