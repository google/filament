fn min_84c9fe() {
  var res = min(1, 1);
}

@fragment
fn fragment_main() {
  min_84c9fe();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_84c9fe();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  min_84c9fe();
  return out;
}
