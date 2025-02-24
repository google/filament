var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var one : f32;
  one = 1.0;
  let x_21 : f32 = one;
  if ((dot(vec2<f32>(2.0, 1.0), vec2<f32>(1.0, select(x_21, 0.0, true))) != 2.0)) {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  } else {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  }
  return;
}

struct main_out {
  @location(0)
  x_GLF_color_1 : vec4<f32>,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(x_GLF_color);
}
