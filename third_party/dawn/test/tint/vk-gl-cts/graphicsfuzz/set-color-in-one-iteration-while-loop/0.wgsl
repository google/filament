struct buf0 {
  injectionSwitch : vec2<f32>,
}

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_6 : buf0;

fn main_1() {
  loop {
    var x_42 : vec4<f32>;
    var x_39 : bool;
    var x_38_phi : bool;
    var x_41_phi : vec4<f32>;
    let x_32 : f32 = gl_FragCoord.x;
    let x_34 : i32 = i32(clamp(x_32, 0.0, 1.0));
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
    x_38_phi = false;
    x_41_phi = vec4<f32>(0.0, 0.0, 0.0, 0.0);
    loop {
      var x_42_phi : vec4<f32>;
      var x_47_phi : i32;
      var x_39_phi : bool;
      let x_38 : bool = x_38_phi;
      let x_41 : vec4<f32> = x_41_phi;
      x_42_phi = x_41;
      x_47_phi = 0;
      loop {
        var x_45 : vec4<f32>;
        var x_48 : i32;
        x_42 = x_42_phi;
        let x_47 : i32 = x_47_phi;
        let x_50 : f32 = x_6.injectionSwitch.y;
        x_39_phi = x_38;
        if ((x_47 < (x_34 + i32(x_50)))) {
        } else {
          break;
        }
        var x_66 : vec4<f32>;
        var x_70 : vec4<f32>;
        var x_45_phi : vec4<f32>;
        if ((x_34 < 0)) {
          x_39_phi = true;
          break;
        } else {
          if ((x_34 == 1)) {
            let x_64 : f32 = f32(x_34);
            let x_65 : vec2<f32> = vec2<f32>(x_64, x_64);
            x_66 = vec4<f32>(x_65.x, x_42.y, x_42.z, x_65.y);
            x_45_phi = x_66;
          } else {
            let x_68 : f32 = f32((x_34 + 1));
            let x_69 : vec2<f32> = vec2<f32>(x_68, x_68);
            x_70 = vec4<f32>(x_69.x, x_42.y, x_42.z, x_69.y);
            x_45_phi = x_70;
          }
          x_45 = x_45_phi;
        }

        continuing {
          x_48 = (x_47 + 1);
          x_42_phi = x_45;
          x_47_phi = x_48;
        }
      }
      x_39 = x_39_phi;
      if (x_39) {
        break;
      }

      continuing {
        x_38_phi = x_39;
        x_41_phi = x_42;
        break if !(x_34 < 0);
      }
    }
    if (x_39) {
      break;
    }
    x_GLF_color = x_42;
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
