enable f16;

@fragment
fn main0() -> @location(0) i32 {
  return 1;
}

@fragment
fn main1() -> @location(1) u32 {
  return 1u;
}

@fragment
fn main2() -> @location(2) f32 {
  return 1.0;
}

@fragment
fn main3() -> @location(3) vec4<f32> {
  return vec4<f32>(1.0, 2.0, 3.0, 4.0);
}

@fragment
fn main4() -> @location(4) f16 {
  return 2.25h;
}

@fragment
fn main5() -> @location(5) vec3<f16> {
  return vec3<f16>(3.0h, 5.0h, 8.0h);
}
