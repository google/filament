var<private> x_2 : vec4<f32>;

var<private> x_3 : i32;

var<private> x_4 : i32;

@group(0) @binding(0) var x_5 : texture_storage_2d<r32sint, write>;

fn main_1() {
  x_4 = 1;
  let x_23 : vec4<f32> = x_2;
  let x_27 : i32 = i32(x_23.x);
  let x_28 : i32 = i32(x_23.y);
  let x_33 : i32 = x_3;
  if (((((x_27 & 1) + (x_28 & 1)) + x_33) == i32(x_23.z))) {
  }
  textureStore(x_5, vec2<i32>(x_27, x_28), vec4<i32>(x_27, 0, 0, 0));
  return;
}

struct main_out {
  @location(0) @interpolate(flat)
  x_4_1 : i32,
}

@fragment
fn main(@builtin(position) x_2_param : vec4<f32>, @location(0) @interpolate(flat) x_3_param : i32) -> main_out {
  x_2 = x_2_param;
  x_3 = x_3_param;
  main_1();
  return main_out(x_4);
}
