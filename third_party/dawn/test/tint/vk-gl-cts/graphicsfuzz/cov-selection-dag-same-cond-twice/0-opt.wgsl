struct buf0 {
  one : f32,
}

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_5 : buf0;

fn main_1() {
  var x_29 : bool;
  var x_30_phi : bool;
  x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  let x_23 : f32 = x_5.one;
  let x_24 : bool = (x_23 < 0.0);
  x_30_phi = x_24;
  if (x_24) {
    let x_28 : f32 = x_5.one;
    x_29 = (x_28 < 0.0);
    x_30_phi = x_29;
  }
  let x_30 : bool = x_30_phi;
  if (x_30) {
    x_GLF_color = vec4<f32>(1.0, 1.0, 1.0, 1.0);
  } else {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
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
