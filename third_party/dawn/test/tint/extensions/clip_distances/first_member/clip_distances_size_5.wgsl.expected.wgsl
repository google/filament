enable clip_distances;

struct VertexOutputs {
  @builtin(clip_distances)
  clipDistance : array<f32, 5>,
  @builtin(position)
  position : vec4<f32>,
}

@vertex
fn main() -> VertexOutputs {
  return VertexOutputs(array<f32, 5>(0.0, 0.0, 0.0, 0.0, 0.0), vec4<f32>(1.0, 2.0, 3.0, 4.0));
}
