struct buf0 {
  injectionSwitch : vec2<f32>,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn func_() -> f32 {
  var x : i32;
  let x_99 : f32 = gl_FragCoord.x;
  if ((x_99 < 1.0)) {
    return 5.0;
  }
  let x_104 : f32 = x_7.injectionSwitch.x;
  let x_106 : f32 = x_7.injectionSwitch.y;
  if ((x_104 > x_106)) {
    return 1.0;
  }
  let x_111 : f32 = x_7.injectionSwitch.x;
  x = i32(x_111);
  let x_114 : f32 = x_7.injectionSwitch.x;
  let x_118 : i32 = x;
  x = (x_118 + (i32(clamp(x_114, 0.0, 1.0)) * 3));
  let x_120 : i32 = x;
  return (5.0 + f32(x_120));
}

fn main_1() {
  var i : i32;
  var j : i32;
  var data : array<vec2<f32>, 17u>;
  i = 0;
  loop {
    let x_48 : i32 = i;
    let x_50 : f32 = x_7.injectionSwitch.x;
    if ((x_48 < (4 + i32(x_50)))) {
    } else {
      break;
    }
    let x_56 : f32 = gl_FragCoord.x;
    if ((x_56 >= 0.0)) {
      j = 0;
      loop {
        var x_81 : bool;
        var x_82_phi : bool;
        let x_64 : i32 = j;
        if ((x_64 < 4)) {
        } else {
          break;
        }
        let x_67 : i32 = j;
        let x_69 : i32 = i;
        let x_71 : f32 = func_();
        data[((4 * x_67) + x_69)].x = x_71;
        let x_74 : f32 = data[0].x;
        let x_75 : bool = (x_74 == 5.0);
        x_82_phi = x_75;
        if (!(x_75)) {
          let x_80 : f32 = data[15].x;
          x_81 = (x_80 == 5.0);
          x_82_phi = x_81;
        }
        let x_82 : bool = x_82_phi;
        if (x_82) {
          x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
        } else {
          x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
        }
        let x_87 : f32 = x_7.injectionSwitch.x;
        let x_89 : f32 = x_7.injectionSwitch.y;
        if ((x_87 > x_89)) {
          return;
        }

        continuing {
          let x_93 : i32 = j;
          j = (x_93 + 1);
        }
      }
    }

    continuing {
      let x_95 : i32 = i;
      i = (x_95 + 1);
    }
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
