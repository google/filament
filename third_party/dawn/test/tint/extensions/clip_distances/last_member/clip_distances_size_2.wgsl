enable clip_distances;

struct VertexOutputs {
  @builtin(position)
  position : vec4<f32>,
  @builtin(clip_distances)
  clipDistance : array<f32, 2>,
}

@vertex
fn main() -> VertexOutputs {
  return VertexOutputs(vec4<f32>(1.0, 2.0, 3.0, 4.0), array<f32, 2>(0.0, 0.0));
}
