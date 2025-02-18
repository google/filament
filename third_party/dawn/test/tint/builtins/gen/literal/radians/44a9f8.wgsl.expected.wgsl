fn radians_44a9f8() {
  var res = radians(vec2(1.0));
}

@fragment
fn fragment_main() {
  radians_44a9f8();
}

@compute @workgroup_size(1)
fn compute_main() {
  radians_44a9f8();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  radians_44a9f8();
  return out;
}
