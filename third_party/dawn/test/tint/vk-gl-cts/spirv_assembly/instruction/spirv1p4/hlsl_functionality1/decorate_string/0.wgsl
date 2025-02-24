var<private> vert_pos : vec4<f32>;

var<private> pos : u32;

var<private> gl_Position : vec4<f32>;

fn main_1() {
  let x_22 : vec4<f32> = vert_pos;
  gl_Position = x_22;
  pos = 0u;
  return;
}

struct main_out {
  @builtin(position)
  gl_Position : vec4<f32>,
  @location(0) @interpolate(flat)
  pos_1 : u32,
}

@vertex
fn main(@location(0) position_param : vec4<f32>) -> main_out {
  vert_pos = position_param;
  main_1();
  return main_out(gl_Position, pos);
}
