var<private> color_out : vec4<f32>;

var<private> color_in : vec4<f32>;

fn main_1() {
  let x_12 : vec4<f32> = color_in;
  color_out = x_12;
  return;
}

struct main_out {
  @location(0)
  color_out_1 : vec4<f32>,
}

@fragment
fn main(@location(0) color_in_param : vec4<f32>) -> main_out {
  color_in = color_in_param;
  main_1();
  return main_out(color_out);
}
