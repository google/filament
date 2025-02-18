fn distance_83911f() {
  var res = distance(vec3(1.0), vec3(1.0));
}

@fragment
fn fragment_main() {
  distance_83911f();
}

@compute @workgroup_size(1)
fn compute_main() {
  distance_83911f();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  distance_83911f();
  return out;
}
