struct buf0 {
  fourtytwo : f32,
}

@group(0) @binding(0) var<uniform> x_5 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var x_36 : bool;
  var x_37_phi : bool;
  let x_23 : f32 = x_5.fourtytwo;
  let x_25 : f32 = x_5.fourtytwo;
  let x_27 : bool = (clamp(1.0, x_23, x_25) > 42.0);
  x_37_phi = x_27;
  if (!(x_27)) {
    let x_32 : f32 = x_5.fourtytwo;
    let x_34 : f32 = x_5.fourtytwo;
    x_36 = (clamp(1.0, x_32, x_34) < 42.0);
    x_37_phi = x_36;
  }
  let x_37 : bool = x_37_phi;
  if (x_37) {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
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
