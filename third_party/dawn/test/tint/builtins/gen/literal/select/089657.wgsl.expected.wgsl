fn select_089657() {
  var res = select(vec3(1.0), vec3(1.0), true);
}

@fragment
fn fragment_main() {
  select_089657();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_089657();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  select_089657();
  return out;
}
