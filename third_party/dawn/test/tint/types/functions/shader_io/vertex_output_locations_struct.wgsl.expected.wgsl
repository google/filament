struct VertexOutputs {
  @location(0) @interpolate(flat)
  loc0 : i32,
  @location(1) @interpolate(flat)
  loc1 : u32,
  @location(2)
  loc2 : f32,
  @location(3)
  loc3 : vec4<f32>,
  @builtin(position)
  position : vec4<f32>,
}

@vertex
fn main() -> VertexOutputs {
  return VertexOutputs(1, 1u, 1.0, vec4<f32>(1.0, 2.0, 3.0, 4.0), vec4<f32>());
}
