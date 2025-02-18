fn smoothstep_66e4bd() {
  const arg_0 = vec3(2.0);
  const arg_1 = vec3(4.0);
  const arg_2 = vec3(3.0);
  var res = smoothstep(arg_0, arg_1, arg_2);
}

@fragment
fn fragment_main() {
  smoothstep_66e4bd();
}

@compute @workgroup_size(1)
fn compute_main() {
  smoothstep_66e4bd();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  smoothstep_66e4bd();
  return out;
}
