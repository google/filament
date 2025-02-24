var<private> final_color : vec4<f32>;

var<private> frag_color : vec4<f32>;

fn main_1() {
  let x_12 : vec4<f32> = frag_color;
  final_color = x_12;
  return;
}

struct main_out {
  @location(0)
  final_color_1 : vec4<f32>,
}

@fragment
fn main(@location(0) frag_color_param : vec4<f32>) -> main_out {
  frag_color = frag_color_param;
  main_1();
  return main_out(final_color);
}
