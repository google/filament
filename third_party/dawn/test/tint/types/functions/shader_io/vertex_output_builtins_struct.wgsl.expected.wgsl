struct VertexOutputs {
  @builtin(position)
  position : vec4<f32>,
}

@vertex
fn main() -> VertexOutputs {
  return VertexOutputs(vec4<f32>(1.0, 2.0, 3.0, 4.0));
}
