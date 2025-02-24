fn cross_1d7933() {
  var res = cross(vec3(1.0), vec3(1.0));
}

@fragment
fn fragment_main() {
  cross_1d7933();
}

@compute @workgroup_size(1)
fn compute_main() {
  cross_1d7933();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  cross_1d7933();
  return out;
}
