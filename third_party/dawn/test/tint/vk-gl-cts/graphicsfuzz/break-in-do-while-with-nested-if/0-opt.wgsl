struct buf0 {
  injectionSwitch : vec2<f32>,
}

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_5 : buf0;

fn main_1() {
  var GLF_live12c5 : bool;
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  loop {
    let x_31 : f32 = x_5.injectionSwitch.y;
    if ((x_31 < 0.0)) {
      GLF_live12c5 = false;
      let x_35 : bool = GLF_live12c5;
      if (x_35) {
        continue;
      } else {
        continue;
      }
    }
    break;

    continuing {
      break if !(false);
    }
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
