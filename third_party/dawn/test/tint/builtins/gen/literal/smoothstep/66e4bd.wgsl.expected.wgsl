fn smoothstep_66e4bd() {
  var res = smoothstep(vec3(2.0), vec3(4.0), vec3(3.0));
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
