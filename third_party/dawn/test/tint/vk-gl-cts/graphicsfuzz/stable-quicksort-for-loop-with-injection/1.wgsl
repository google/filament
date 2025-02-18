var<private> x_GLF_color : vec4<f32>;

var<private> frag_color : vec4<f32>;

fn main_1() {
  let x_12 : vec4<f32> = frag_color;
  x_GLF_color = x_12;
  return;
}

struct main_out {
  @location(0)
  x_GLF_color_1 : vec4<f32>,
}

@fragment
fn main(@location(0) frag_color_param : vec4<f32>) -> main_out {
  frag_color = frag_color_param;
  main_1();
  return main_out(x_GLF_color);
}
