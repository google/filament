struct VertexInputs0 {
  @builtin(vertex_index) vertex_index : u32,
  @location(0) loc0 : i32,
};
struct VertexInputs1 {
  @location(2) loc1 : u32,
  @location(3) loc3 : vec4<f32>,
};

@vertex
fn main(
  inputs0 : VertexInputs0,
  @location(1) loc1 : u32,
  @builtin(instance_index) instance_index : u32,
  inputs1 : VertexInputs1,
) -> @builtin(position) vec4<f32> {
  let foo : u32 = inputs0.vertex_index + instance_index;
  return vec4<f32>();
}
