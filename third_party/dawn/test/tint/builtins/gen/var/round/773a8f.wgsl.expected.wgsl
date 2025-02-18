fn round_773a8f() {
  const arg_0 = 3.5;
  var res = round(arg_0);
}

@fragment
fn fragment_main() {
  round_773a8f();
}

@compute @workgroup_size(1)
fn compute_main() {
  round_773a8f();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  round_773a8f();
  return out;
}
