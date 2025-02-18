struct VertexInputs {
  @builtin(vertex_index)
  vertex_index : u32,
  @builtin(instance_index)
  instance_index : u32,
}

@vertex
fn main(inputs : VertexInputs) -> @builtin(position) vec4<f32> {
  let foo : u32 = (inputs.vertex_index + inputs.instance_index);
  return vec4<f32>();
}
