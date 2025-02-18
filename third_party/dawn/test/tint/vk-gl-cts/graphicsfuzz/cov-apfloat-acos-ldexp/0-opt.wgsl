struct buf0 {
  two : i32,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var v : vec2<f32>;
  var d : f32;
  let x_35 : i32 = x_6.two;
  v = acos(ldexp(vec2<f32>(0.100000001, 0.100000001), vec2<i32>(x_35, 3)));
  let x_39 : vec2<f32> = v;
  d = distance(x_39, vec2<f32>(1.159279943, 0.64349997));
  let x_41 : f32 = d;
  if ((x_41 < 0.01)) {
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
