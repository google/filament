struct buf0 {
  injectionSwitch : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_5 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var x_30 : f32;
  var x_32_phi : f32;
  x_32_phi = 0.0;
  loop {
    var x_33_phi : f32;
    let x_32 : f32 = x_32_phi;
    x_33_phi = x_32;
    let x_33 : f32 = x_33_phi;
    let x_39 : f32 = x_5.injectionSwitch.x;
    let x_41 : f32 = x_5.injectionSwitch.y;
    if ((x_39 < x_41)) {
      x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
      return;
    } else {
      continue;
    }

    continuing {
      x_32_phi = x_33;
    }
  }
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
