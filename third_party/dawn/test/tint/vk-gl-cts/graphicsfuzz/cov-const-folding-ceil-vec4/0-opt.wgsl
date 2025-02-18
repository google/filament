struct buf0 {
  quarter : f32,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var v : vec4<f32>;
  let x_32 : f32 = x_6.quarter;
  v = ceil(vec4<f32>(424.113006592, x_32, 1.299999952, 19.620000839));
  let x_35 : vec4<f32> = v;
  if (all((x_35 == vec4<f32>(425.0, 1.0, 2.0, 20.0)))) {
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
