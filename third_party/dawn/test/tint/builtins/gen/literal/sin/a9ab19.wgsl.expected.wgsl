fn sin_a9ab19() {
  var res = sin(1.57079632679000003037);
}

@fragment
fn fragment_main() {
  sin_a9ab19();
}

@compute @workgroup_size(1)
fn compute_main() {
  sin_a9ab19();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  sin_a9ab19();
  return out;
}
