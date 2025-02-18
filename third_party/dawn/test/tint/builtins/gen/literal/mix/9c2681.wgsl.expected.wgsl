fn mix_9c2681() {
  var res = mix(vec3(1.0), vec3(1.0), 1.0);
}

@fragment
fn fragment_main() {
  mix_9c2681();
}

@compute @workgroup_size(1)
fn compute_main() {
  mix_9c2681();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  mix_9c2681();
  return out;
}
