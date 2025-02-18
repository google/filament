fn select_17441a() {
  var res = select(vec4(1.0), vec4(1.0), true);
}

@fragment
fn fragment_main() {
  select_17441a();
}

@compute @workgroup_size(1)
fn compute_main() {
  select_17441a();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  select_17441a();
  return out;
}
