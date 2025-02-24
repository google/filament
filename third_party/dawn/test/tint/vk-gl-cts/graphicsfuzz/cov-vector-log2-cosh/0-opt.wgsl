var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var v : vec2<f32>;
  v = log2(cosh(vec2<f32>(1.0, 10.0)));
  let x_27 : f32 = v.x;
  let x_29 : f32 = v.y;
  if ((x_27 < x_29)) {
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
