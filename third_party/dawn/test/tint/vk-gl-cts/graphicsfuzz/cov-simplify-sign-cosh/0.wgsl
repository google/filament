struct buf0 {
  one : i32,
}

@group(0) @binding(0) var<uniform> x_5 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var x_31 : bool;
  var x_32_phi : bool;
  let x_26 : bool = (sign(cosh(70.0f)) == 1.0);
  x_32_phi = x_26;
  if (!(x_26)) {
    let x_6 : i32 = x_5.one;
    x_31 = (x_6 == 1);
    x_32_phi = x_31;
  }
  let x_32 : bool = x_32_phi;
  if (x_32) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  } else {
    x_GLF_color = vec4<f32>(1.0, 1.0, 1.0, 1.0);
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
