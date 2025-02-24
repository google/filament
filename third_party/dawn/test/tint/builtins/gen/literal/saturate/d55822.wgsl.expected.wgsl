fn saturate_d55822() {
  var res = saturate(vec3(2.0));
}

@fragment
fn fragment_main() {
  saturate_d55822();
}

@compute @workgroup_size(1)
fn compute_main() {
  saturate_d55822();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  saturate_d55822();
  return out;
}
