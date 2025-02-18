@vertex
fn main(@location(0) loc0 : i32, @location(1) loc1 : u32, @location(2) loc2 : f32, @location(3) loc3 : vec4<f32>) -> @builtin(position) vec4<f32> {
  let i : i32 = loc0;
  let u : u32 = loc1;
  let f : f32 = loc2;
  let v : vec4<f32> = loc3;
  return vec4<f32>();
}
