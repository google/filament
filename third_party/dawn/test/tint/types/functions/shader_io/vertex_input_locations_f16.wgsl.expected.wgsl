enable f16;

@vertex
fn main(@location(0) loc0 : i32, @location(1) loc1 : u32, @location(2) loc2 : f32, @location(3) loc3 : vec4<f32>, @location(4) loc4 : f16, @location(5) loc5 : vec3<f16>) -> @builtin(position) vec4<f32> {
  let i : i32 = loc0;
  let u : u32 = loc1;
  let f : f32 = loc2;
  let v : vec4<f32> = loc3;
  let x : f16 = loc4;
  let y : vec3<f16> = loc5;
  return vec4<f32>();
}
