fn distance_f9c9ee() {
  var res = distance(1.0, 1.0);
}

@fragment
fn fragment_main() {
  distance_f9c9ee();
}

@compute @workgroup_size(1)
fn compute_main() {
  distance_f9c9ee();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  distance_f9c9ee();
  return out;
}
