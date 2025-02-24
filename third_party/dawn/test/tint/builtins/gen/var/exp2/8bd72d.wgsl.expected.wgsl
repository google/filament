fn exp2_8bd72d() {
  const arg_0 = vec4(1.0);
  var res = exp2(arg_0);
}

@fragment
fn fragment_main() {
  exp2_8bd72d();
}

@compute @workgroup_size(1)
fn compute_main() {
  exp2_8bd72d();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  exp2_8bd72d();
  return out;
}
