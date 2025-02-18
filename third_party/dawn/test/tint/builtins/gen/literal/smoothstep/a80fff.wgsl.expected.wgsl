fn smoothstep_a80fff() {
  var res = smoothstep(2.0, 4.0, 3.0);
}

@fragment
fn fragment_main() {
  smoothstep_a80fff();
}

@compute @workgroup_size(1)
fn compute_main() {
  smoothstep_a80fff();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  smoothstep_a80fff();
  return out;
}
