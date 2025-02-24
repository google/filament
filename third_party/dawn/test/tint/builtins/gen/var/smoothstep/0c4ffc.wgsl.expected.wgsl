fn smoothstep_0c4ffc() {
  const arg_0 = vec4(2.0);
  const arg_1 = vec4(4.0);
  const arg_2 = vec4(3.0);
  var res = smoothstep(arg_0, arg_1, arg_2);
}

@fragment
fn fragment_main() {
  smoothstep_0c4ffc();
}

@compute @workgroup_size(1)
fn compute_main() {
  smoothstep_0c4ffc();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  smoothstep_0c4ffc();
  return out;
}
