var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var v1 : vec2<f32>;
  var v2 : vec2<f32>;
  v1 = vec2<f32>(1.0, -1.0);
  let x_22 : vec2<f32> = v1;
  v2 = step(vec2<f32>(0.400000006, 0.400000006), sinh(x_22));
  let x_27 : f32 = v2.x;
  let x_29 : f32 = v2.y;
  let x_31 : f32 = v2.y;
  let x_33 : f32 = v2.x;
  x_GLF_color = vec4<f32>(x_27, x_29, x_31, x_33);
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
