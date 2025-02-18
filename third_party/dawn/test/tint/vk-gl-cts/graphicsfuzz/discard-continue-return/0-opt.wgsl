struct buf0 {
  injectionSwitch : vec2<f32>,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  loop {
    var x_46_phi : bool;
    loop {
      let x_37 : f32 = gl_FragCoord.x;
      if ((x_37 < 0.0)) {
        let x_42 : f32 = x_6.injectionSwitch.y;
        if ((1.0 > x_42)) {
          discard;
        } else {
          continue;
        }
      }
      x_46_phi = true;
      break;

      continuing {
        x_46_phi = false;
        break if !(false);
      }
    }
    let x_46 : bool = x_46_phi;
    if (x_46) {
      break;
    }
    break;
  }
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  return;
}

struct main_out {
  @location(0)
  x_GLF_color_1 : vec4<f32>,
}

@fragment
fn main(@builtin(position) gl_FragCoord_param : vec4<f32>) -> main_out {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  return main_out(x_GLF_color);
}
