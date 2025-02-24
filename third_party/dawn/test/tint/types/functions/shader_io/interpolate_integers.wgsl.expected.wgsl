struct Interface {
  @location(0) @interpolate(flat)
  i : i32,
  @location(1) @interpolate(flat)
  u : u32,
  @location(2) @interpolate(flat)
  vi : vec4<i32>,
  @location(3) @interpolate(flat)
  vu : vec4<u32>,
  @builtin(position)
  pos : vec4<f32>,
}

@vertex
fn vert_main() -> Interface {
  return Interface();
}

@fragment
fn frag_main(inputs : Interface) -> @location(0) i32 {
  return inputs.i;
}
