struct buf0 {
  injectionSwitch : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_9 : buf0;

var<private> x_GLF_color : vec4<f32>;

var<private> gl_FragCoord : vec4<f32>;

fn main_1() {
  var v : vec2<f32>;
  var floats : array<f32, 9u>;
  var one : i32;
  var i : i32;
  var alwaysFalse : bool;
  v = vec2<f32>(0.0, 0.0);
  floats[1] = 0.0;
  let x_46 : f32 = x_9.injectionSwitch.y;
  one = i32(x_46);
  loop {
    i = 0;
    loop {
      let x_56 : i32 = i;
      let x_57 : i32 = one;
      if ((x_56 < x_57)) {
      } else {
        break;
      }
      let x_60 : i32 = i;
      if ((x_60 == 0)) {
        let x_65 : f32 = x_9.injectionSwitch.x;
        let x_67 : f32 = x_9.injectionSwitch.y;
        alwaysFalse = (x_65 > x_67);
        let x_69 : bool = alwaysFalse;
        if (!(x_69)) {
          let x_73 : i32 = one;
          floats[x_73] = 1.0;
          x_GLF_color = vec4<f32>(1.0, 1.0, 0.0, 1.0);
        }
        let x_75 : i32 = one;
        v[x_75] = 1.0;
        let x_77 : bool = alwaysFalse;
        if (x_77) {
          discard;
        }
        let x_81 : f32 = x_9.injectionSwitch.y;
        if ((x_81 < 0.0)) {
          x_GLF_color = vec4<f32>(0.0, 1.0, 0.0, 1.0);
        }
      }

      continuing {
        let x_85 : i32 = i;
        i = (x_85 + 1);
      }
    }

    continuing {
      let x_87 : i32 = one;
      break if !(x_87 < 0);
    }
  }
  var x_102 : bool;
  var x_103_phi : bool;
  let x_90 : f32 = gl_FragCoord.y;
  if ((x_90 >= 0.0)) {
    let x_96 : f32 = v.y;
    let x_97 : bool = (x_96 == 1.0);
    x_103_phi = x_97;
    if (x_97) {
      let x_101 : f32 = floats[1];
      x_102 = (x_101 == 1.0);
      x_103_phi = x_102;
    }
    let x_103 : bool = x_103_phi;
    if (x_103) {
      x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
    }
  } else {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
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
