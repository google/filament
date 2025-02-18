enable f16;

struct FragmentOutputs {
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

@fragment
fn main() -> FragmentOutputs {
  return FragmentOutputs(1, 1u, 1.0, vec4<f32>(1.0, 2.0, 3.0, 4.0), 2.25h, vec3<f16>(3.0h, 5.0h, 8.0h));
}
