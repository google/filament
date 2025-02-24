fn degrees_fafa7e() {
  var res = degrees(1.0);
}

@fragment
fn fragment_main() {
  degrees_fafa7e();
}

@compute @workgroup_size(1)
fn compute_main() {
  degrees_fafa7e();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  degrees_fafa7e();
  return out;
}
