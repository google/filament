fn distance_3a175a() {
  var res = distance(vec2(1.0), vec2(1.0));
}

@fragment
fn fragment_main() {
  distance_3a175a();
}

@compute @workgroup_size(1)
fn compute_main() {
  distance_3a175a();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  distance_3a175a();
  return out;
}
