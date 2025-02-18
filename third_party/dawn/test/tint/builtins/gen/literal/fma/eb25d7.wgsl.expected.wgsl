fn fma_eb25d7() {
  var res = fma(vec3(1.0), vec3(1.0), vec3(1.0));
}

@fragment
fn fragment_main() {
  fma_eb25d7();
}

@compute @workgroup_size(1)
fn compute_main() {
  fma_eb25d7();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  fma_eb25d7();
  return out;
}
