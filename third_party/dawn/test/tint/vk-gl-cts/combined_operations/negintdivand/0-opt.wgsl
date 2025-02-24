var<private> pos : vec4<f32>;

var<private> frag_color : vec4<f32>;

var<private> gl_Position : vec4<f32>;

fn main_1() {
  let x_21 : vec4<f32> = pos;
  gl_Position = x_21;
  let x_23 : vec4<f32> = pos;
  frag_color = (x_23 * 0.5);
  return;
}

struct main_out {
  @builtin(position)
  gl_Position : vec4<f32>,
  @location(1)
  frag_color_1 : vec4<f32>,
}

@vertex
fn main(@location(0) position_param : vec4<f32>) -> main_out {
  pos = position_param;
  main_1();
  return main_out(gl_Position, frag_color);
}
