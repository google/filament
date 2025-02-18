var<private> pos : vec4<f32>;

var<private> gl_Position : vec4<f32>;

fn main_1() {
  let x_22 : vec4<f32> = pos;
  let x_23 : vec2<f32> = vec2<f32>(x_22.x, x_22.y);
  gl_Position = vec4<f32>(x_23.x, x_23.y, 0.600000024, 1.0);
  return;
}

struct main_out {
  @builtin(position)
  gl_Position : vec4<f32>,
}

@vertex
fn main(@location(0) position_param : vec4<f32>) -> main_out {
  pos = position_param;
  main_1();
  return main_out(gl_Position);
}
