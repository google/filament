fn mix_343c49() {
  var res = mix(vec3(1.0), vec3(1.0), vec3(1.0));
}

@fragment
fn fragment_main() {
  mix_343c49();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_343c49();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  mix_343c49();
  return out;
}
