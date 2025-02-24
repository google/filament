struct VertexInputs {
  @location(0) loc0 : i32,
  @location(1) loc1 : u32,
  @location(2) loc2 : f32,
  @location(3) loc3 : vec4<f32>,
};

@vertex
fn main(inputs : VertexInputs) -> @builtin(position) vec4<f32> {
  let i : i32 = inputs.loc0;
  let u : u32 = inputs.loc1;
  let f : f32 = inputs.loc2;
  let v : vec4<f32> = inputs.loc3;
  return vec4<f32>();
}
