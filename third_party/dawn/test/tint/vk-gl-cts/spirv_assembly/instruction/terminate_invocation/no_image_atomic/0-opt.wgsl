var<private> x_2 : vec3<f32>;

var<private> x_3 : i32;

var<private> x_4 : i32;

var<private> gl_Position : vec4<f32>;

fn main_1() {
  let x_22 : vec3<f32> = x_2;
  gl_Position = vec4<f32>(x_22, 1.0);
  x_4 = x_3;
  return;
}

struct main_out {
  @location(0) @interpolate(flat)
  x_4_1 : i32,
  @builtin(position)
  gl_Position : vec4<f32>,
}

@vertex
fn main(@location(0) x_2_param : vec3<f32>, @location(1) @interpolate(flat) x_3_param : i32) -> main_out {
  x_2 = x_2_param;
  x_3 = x_3_param;
  main_1();
  return main_out(x_4, gl_Position);
}
