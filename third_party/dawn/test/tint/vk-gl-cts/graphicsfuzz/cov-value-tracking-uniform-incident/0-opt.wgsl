struct buf0 {
  quarter : f32,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var N : vec4<f32>;
  var I : vec4<f32>;
  var Nref : vec4<f32>;
  var v : vec4<f32>;
  N = vec4<f32>(1.0, 2.0, 3.0, 4.0);
  let x_44 : f32 = x_7.quarter;
  I = vec4<f32>(4.0, 87.589996338, x_44, 92.510002136);
  Nref = vec4<f32>(17.049999237, -6.099999905, 4329.370605469, 2.700000048);
  let x_46 : vec4<f32> = N;
  let x_47 : vec4<f32> = I;
  let x_48 : vec4<f32> = Nref;
  v = faceForward(x_46, x_47, x_48);
  let x_50 : vec4<f32> = v;
  if (all((x_50 == vec4<f32>(-1.0, -2.0, -3.0, -4.0)))) {
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
