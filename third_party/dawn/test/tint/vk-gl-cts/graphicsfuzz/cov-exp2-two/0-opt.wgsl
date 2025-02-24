var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var f : f32;
  f = 2.0;
  let x_19 : f32 = f;
  if ((exp2(x_19) == 4.0)) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  } else {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
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
