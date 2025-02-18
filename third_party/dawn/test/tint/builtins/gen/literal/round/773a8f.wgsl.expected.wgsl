fn round_773a8f() {
  var res = round(3.5);
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
