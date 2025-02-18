struct buf0 {
  zero : i32,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : vec2<f32>;
  var b : vec2<f32>;
  a = vec2<f32>(1.0, 1.0);
  let x_38 : i32 = x_6.zero;
  if ((x_38 == 1)) {
    let x_43 : f32 = a.x;
    a.x = (x_43 + 1.0);
  }
  let x_47 : f32 = a.y;
  b = (vec2<f32>(x_47, x_47) + vec2<f32>(2.0, 3.0));
  let x_50 : vec2<f32> = b;
  if (all((x_50 == vec2<f32>(3.0, 4.0)))) {
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
