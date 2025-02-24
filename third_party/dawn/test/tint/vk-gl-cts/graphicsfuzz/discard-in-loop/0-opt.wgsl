var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn x_47() {
  discard;
}

fn main_1() {
  loop {
    var x_30_phi : i32;
    var x_48_phi : bool;
    x_30_phi = 0;
    loop {
      var x_31 : i32;
      let x_30 : i32 = x_30_phi;
      x_48_phi = false;
      if ((x_30 < 10)) {
      } else {
        break;
      }
      let x_37 : f32 = gl_FragCoord.y;
      if ((x_37 < 0.0)) {
        let x_42 : f32 = gl_FragCoord.x;
        if ((x_42 < 0.0)) {
          x_48_phi = false;
          break;
        } else {
          continue;
        }
      }
      x_47();

      continuing {
        x_31 = (x_30 + 1);
        x_30_phi = x_31;
      }
    }
    let x_48 : bool = x_48_phi;
    if (x_48) {
      break;
    }
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
    break;
  }
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
