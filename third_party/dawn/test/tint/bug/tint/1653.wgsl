const TRI_VERTICES = array(
  vec4(0., 0., 0., 1.),
  vec4(0., 1., 0., 1.),
  vec4(1., 1., 0., 1.),
);

@vertex
fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> @builtin(position) vec4<f32> {
  return TRI_VERTICES[in_vertex_index];
}
