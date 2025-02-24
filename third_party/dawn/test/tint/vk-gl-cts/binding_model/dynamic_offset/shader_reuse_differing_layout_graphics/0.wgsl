struct block0 {
  in_color : vec4<f32>,
}

var<private> pos : vec4<f32>;

var<private> frag_color : vec4<f32>;

@group(0) @binding(1) var<uniform> x_8 : block0;

var<private> gl_Position : vec4<f32>;

fn main_1() {
  let x_24 : vec4<f32> = pos;
  gl_Position = x_24;
  let x_27 : vec4<f32> = x_8.in_color;
  frag_color = x_27;
  return;
}

struct main_out {
  @builtin(position)
  gl_Position : vec4<f32>,
  @location(0)
  frag_color_1 : vec4<f32>,
}

@vertex
fn main(@location(0) position_param : vec4<f32>) -> main_out {
  pos = position_param;
  main_1();
  return main_out(gl_Position, frag_color);
}
