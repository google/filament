struct buf0 {
  two : f32,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn func_vf2_(v : ptr<function, vec2<f32>>) -> f32 {
  let x_42 : f32 = x_7.two;
  (*(v)).x = x_42;
  let x_45 : f32 = (*(v)).y;
  if ((x_45 < 1.0)) {
    return 1.0;
  }
  return 5.0;
}

fn main_1() {
  var f : f32;
  var param : vec2<f32>;
  param = vec2<f32>(1.0, 1.0);
  let x_34 : f32 = func_vf2_(&(param));
  f = x_34;
  let x_35 : f32 = f;
  if ((x_35 == 5.0)) {
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
