var<private> x_GLF_color : vec4<f32>;

fn func_() -> i32 {
  var coord : vec2<f32>;
  var tmp3 : f32;
  var tmp2 : array<f32, 1u>;
  var tmp : vec4<f32>;
  var x_48 : f32;
  coord = vec2<f32>(1.0, 1.0);
  let x_41 : f32 = coord.y;
  if ((i32(x_41) < 180)) {
    x_48 = tmp2[0];
    tmp3 = x_48;
  } else {
    discard;
  }
  tmp = vec4<f32>(x_48, x_48, x_48, x_48);
  return 1;
}

fn main_1() {
  let x_9 : i32 = func_();
  if ((x_9 == 1)) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  } else {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 1.0);
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
