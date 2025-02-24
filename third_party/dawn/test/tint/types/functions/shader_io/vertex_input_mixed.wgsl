struct VertexInputs0 {
  @builtin(vertex_index) vertex_index : u32,
  @location(0) loc0 : i32,
};
struct VertexInputs1 {
  @location(2) loc2 : f32,
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
  let i : i32 = inputs0.loc0;
  let u : u32 = loc1;
  let f : f32 = inputs1.loc2;
  let v : vec4<f32> = inputs1.loc3;
  return vec4<f32>();
}
