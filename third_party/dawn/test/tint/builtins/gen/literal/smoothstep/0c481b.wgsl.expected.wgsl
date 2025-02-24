fn smoothstep_0c481b() {
  var res = smoothstep(vec2(2.0), vec2(4.0), vec2(3.0));
}

@fragment
fn fragment_main() {
  smoothstep_0c481b();
}

@compute @workgroup_size(1)
fn compute_main() {
  smoothstep_0c481b();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  smoothstep_0c481b();
  return out;
}
