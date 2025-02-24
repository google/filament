var<private> pos : vec3<f32>;

var<private> gl_Position : vec4<f32>;

fn main_1() {
  let x_21 : vec3<f32> = pos;
  gl_Position = vec4<f32>(x_21.x, x_21.y, x_21.z, 1.0);
  return;
}

struct main_out {
  @builtin(position)
  gl_Position : vec4<f32>,
}

@vertex
fn main(@location(0) position_param : vec3<f32>) -> main_out {
  pos = position_param;
  main_1();
  return main_out(gl_Position);
}
