fn degrees_d43a49() {
  var res = degrees(vec4(1.0));
}

@fragment
fn fragment_main() {
  degrees_d43a49();
}

@compute @workgroup_size(1)
fn compute_main() {
  degrees_d43a49();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  degrees_d43a49();
  return out;
}
