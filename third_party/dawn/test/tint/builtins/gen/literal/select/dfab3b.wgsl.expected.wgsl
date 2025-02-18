fn select_dfab3b() {
  var res = select(vec3(1), vec3(1), true);
}

@fragment
fn fragment_main() {
  select_dfab3b();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_dfab3b();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  select_dfab3b();
  return out;
}
