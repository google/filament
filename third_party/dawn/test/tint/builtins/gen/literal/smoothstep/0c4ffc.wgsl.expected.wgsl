fn smoothstep_0c4ffc() {
  var res = smoothstep(vec4(2.0), vec4(4.0), vec4(3.0));
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
