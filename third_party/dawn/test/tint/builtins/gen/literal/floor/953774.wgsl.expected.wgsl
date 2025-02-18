fn floor_953774() {
  var res = floor(vec3(1.5));
}

@fragment
fn fragment_main() {
  floor_953774();
}

@compute @workgroup_size(1)
fn compute_main() {
  floor_953774();
}

struct VertexOutput {
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vertex_main() -> VertexOutput {
  var out : VertexOutput;
  out.pos = vec4<f32>();
  floor_953774();
  return out;
}
