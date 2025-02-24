var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var v : vec4<f32>;
  let x_23 : vec4<f32> = v;
  v = vec4<f32>(vec2<f32>(1.0, 1.0).x, vec2<f32>(1.0, 1.0).y, x_23.z, x_23.w);
  let x_25 : vec4<f32> = v;
  v = vec4<f32>(x_25.x, x_25.y, vec2<f32>(2.0, 2.0).x, vec2<f32>(2.0, 2.0).y);
  let x_27 : vec4<f32> = v;
  if (all((x_27 == vec4<f32>(1.0, 1.0, 2.0, 2.0)))) {
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
