struct buf1 {
  resolution : vec2<f32>,
}

struct buf0 {
  injectionSwitch : vec2<f32>,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(1) var<uniform> x_6 : buf1;

@group(0) @binding(0) var<uniform> x_9 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var data : array<f32, 10u>;
  var x_40_phi : i32;
  var x_52_phi : i32;
  x_40_phi = 0;
  loop {
    var x_41 : i32;
    let x_40 : i32 = x_40_phi;
    if ((x_40 < 10)) {
    } else {
      break;
    }

    continuing {
      let x_48 : f32 = x_9.injectionSwitch.y;
      data[x_40] = (f32((10 - x_40)) * x_48);
      x_41 = (x_40 + 1);
      x_40_phi = x_41;
    }
  }
  x_52_phi = 0;
  loop {
    var x_53 : i32;
    var x_59_phi : i32;
    let x_52 : i32 = x_52_phi;
    if ((x_52 < 9)) {
    } else {
      break;
    }
    x_59_phi = 0;
    loop {
      var x_82 : bool;
      var x_83 : bool;
      var x_60 : i32;
      var x_84_phi : bool;
      let x_59 : i32 = x_59_phi;
      if ((x_59 < 10)) {
      } else {
        break;
      }
      if ((x_59 < (x_52 + 1))) {
        continue;
      }
      let x_69_save = x_52;
      let x_70 : f32 = data[x_69_save];
      let x_71_save = x_59;
      let x_72 : f32 = data[x_71_save];
      let x_74 : f32 = gl_FragCoord.y;
      let x_76 : f32 = x_6.resolution.y;
      if ((x_74 < (x_76 * 0.5))) {
        x_82 = (x_70 > x_72);
        x_84_phi = x_82;
      } else {
        x_83 = (x_70 < x_72);
        x_84_phi = x_83;
      }
      let x_84 : bool = x_84_phi;
      if (x_84) {
        let x_87 : f32 = data[x_69_save];
        let x_88 : f32 = data[x_71_save];
        data[x_69_save] = x_88;
        data[x_71_save] = x_87;
      }

      continuing {
        x_60 = (x_59 + 1);
        x_59_phi = x_60;
      }
    }

    continuing {
      x_53 = (x_52 + 1);
      x_52_phi = x_53;
    }
  }
  let x_90 : f32 = gl_FragCoord.x;
  let x_92 : f32 = x_6.resolution.x;
  if ((x_90 < (x_92 * 0.5))) {
    let x_99 : f32 = data[0];
    let x_102 : f32 = data[5];
    let x_105 : f32 = data[9];
    x_GLF_color = vec4<f32>((x_99 * 0.100000001), (x_102 * 0.100000001), (x_105 * 0.100000001), 1.0);
  } else {
    let x_109 : f32 = data[5];
    let x_112 : f32 = data[9];
    let x_115 : f32 = data[0];
    x_GLF_color = vec4<f32>((x_109 * 0.100000001), (x_112 * 0.100000001), (x_115 * 0.100000001), 1.0);
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
