fn min_794711() {
  const arg_0 = 1.0;
  const arg_1 = 1.0;
  var res = min(arg_0, arg_1);
}

@fragment
fn fragment_main() {
  min_794711();
}

@compute @workgroup_size(1)
fn compute_main() {
  min_794711();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  min_794711();
  return out;
}
