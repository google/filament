var<private> x_2 : vec4<f32>;

var<private> x_3 : i32;

var<private> x_4 : i32;

fn main_1() {
  let x_16 : vec4<f32> = x_2;
  let x_26 : i32 = x_3;
  if (((((i32(x_16.x) & 1) + (i32(x_16.y) & 1)) + x_26) == i32(x_16.z))) {
  }
  x_4 = 1;
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
