enable f16;

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
  @location(4)
  loc4 : f16,
  @location(5)
  loc5 : vec3<f16>,
}

@vertex
fn main() -> VertexOutputs {
  return VertexOutputs(1, 1u, 1.0, vec4<f32>(1.0, 2.0, 3.0, 4.0), vec4<f32>(), 2.25h, vec3<f16>(3.0h, 5.0h, 8.0h));
}
