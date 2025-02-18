fn saturate_e40fb6() {
  const arg_0 = vec2(2.0);
  var res = saturate(arg_0);
}

@fragment
fn fragment_main() {
  saturate_e40fb6();
}

@compute @workgroup_size(1)
fn compute_main() {
  saturate_e40fb6();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  saturate_e40fb6();
  return out;
}
