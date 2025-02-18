fn exp2_303753() {
  const arg_0 = vec3(1.0);
  var res = exp2(arg_0);
}

@fragment
fn fragment_main() {
  exp2_303753();
}

@compute @workgroup_size(1)
fn compute_main() {
  exp2_303753();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  exp2_303753();
  return out;
}
