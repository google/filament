enable f16;

struct VertexInputs {
  @location(0)
  loc0 : i32,
  @location(1)
  loc1 : u32,
  @location(2)
  loc2 : f32,
  @location(3)
  loc3 : vec4<f32>,
  @location(4)
  loc4 : f16,
  @location(5)
  loc5 : vec3<f16>,
}

@vertex
fn main(inputs : VertexInputs) -> @builtin(position) vec4<f32> {
  let i : i32 = inputs.loc0;
  let u : u32 = inputs.loc1;
  let f : f32 = inputs.loc2;
  let v : vec4<f32> = inputs.loc3;
  let x : f16 = inputs.loc4;
  let y : vec3<f16> = inputs.loc5;
  return vec4<f32>();
}
