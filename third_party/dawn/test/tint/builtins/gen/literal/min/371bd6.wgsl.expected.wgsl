fn min_371bd6() {
  var res = min(vec3(1), vec3(1));
}

@fragment
fn fragment_main() {
  min_371bd6();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_371bd6();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  min_371bd6();
  return out;
}
