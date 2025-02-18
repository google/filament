var<private> x_3 : vec2<f32>;

var<private> x_4 : u32;

var<private> gl_Position : vec4<f32>;

fn main_1() {
  let x_30 : f32 = x_3.x;
  let x_36 : f32 = x_3.y;
  x_4 = (u32((((x_30 + 1.027777791) * 18.0) - 1.0)) + (u32((((x_36 + 1.027777791) * 18.0) - 1.0)) * 36u));
  let x_43 : vec2<f32> = x_3;
  gl_Position = vec4<f32>(x_43.x, x_43.y, 0.0, 1.0);
  return;
}

struct main_out {
  @location(0) @interpolate(flat)
  x_4_1 : u32,
  @builtin(position)
  gl_Position : vec4<f32>,
}

@vertex
fn main(@location(0) x_3_param : vec2<f32>) -> main_out {
  x_3 = x_3_param;
  main_1();
  return main_out(x_4, gl_Position);
}
