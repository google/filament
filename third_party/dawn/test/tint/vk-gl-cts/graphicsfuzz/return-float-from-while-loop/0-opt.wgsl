struct buf0 {
  injectionSwitch : vec2<f32>,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var x_30 : bool;
  var x_47 : f32;
  let x_29 : f32 = x_6.injectionSwitch.x;
  x_30 = (x_29 > 1.0);
  if (x_30) {
    loop {
      var x_47_phi : f32;
      loop {
        let x_41 : f32 = gl_FragCoord.x;
        if ((x_41 < 0.0)) {
          if (x_30) {
            x_47_phi = 1.0;
            break;
          } else {
            continue;
          }
        }
        x_47_phi = 0.0;
        break;
      }
      x_47 = x_47_phi;
      break;
    }
    var x_48_1 : vec4<f32> = vec4<f32>();
    x_48_1.y = x_47;
    let x_48 : vec4<f32> = x_48_1;
    x_GLF_color = x_48;
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
