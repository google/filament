fn inverseSqrt_07a6fe() {
  const arg_0 = vec4(1.0);
  var res = inverseSqrt(arg_0);
}

@fragment
fn fragment_main() {
  inverseSqrt_07a6fe();
}

@compute @workgroup_size(1)
fn compute_main() {
  inverseSqrt_07a6fe();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  inverseSqrt_07a6fe();
  return out;
}
