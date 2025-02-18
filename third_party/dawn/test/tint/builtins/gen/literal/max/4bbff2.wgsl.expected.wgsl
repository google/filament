fn max_4bbff2() {
  var res = max(vec4(1.0), vec4(1.0));
}

@fragment
fn fragment_main() {
  max_4bbff2();
}

@compute @workgroup_size(1)
fn compute_main() {
  max_4bbff2();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  max_4bbff2();
  return out;
}
