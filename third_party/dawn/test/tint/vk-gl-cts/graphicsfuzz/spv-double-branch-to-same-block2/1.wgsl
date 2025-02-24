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
  var x_41_phi : i32;
  var x_53_phi : i32;
  x_41_phi = 0;
  loop {
    var x_42 : i32;
    let x_41 : i32 = x_41_phi;
    if ((x_41 < 10)) {
    } else {
      break;
    }

    continuing {
      let x_49 : f32 = x_9.injectionSwitch.y;
      data[x_41] = (f32((10 - x_41)) * x_49);
      x_42 = (x_41 + 1);
      x_41_phi = x_42;
    }
  }
  x_53_phi = 0;
  loop {
    var x_54 : i32;
    var x_60_phi : i32;
    let x_53 : i32 = x_53_phi;
    if ((x_53 < 9)) {
    } else {
      break;
    }
    x_60_phi = 0;
    loop {
      var x_83 : bool;
      var x_84 : bool;
      var x_61 : i32;
      var x_85_phi : bool;
      let x_60 : i32 = x_60_phi;
      if ((x_60 < 10)) {
      } else {
        break;
      }
      if ((x_60 < (x_53 + 1))) {
        continue;
      }
      let x_70_save = x_53;
      let x_71 : f32 = data[x_70_save];
      let x_72_save = x_60;
      let x_73 : f32 = data[x_72_save];
      let x_75 : f32 = gl_FragCoord.y;
      let x_77 : f32 = x_6.resolution.y;
      if ((x_75 < (x_77 * 0.5))) {
        x_83 = (x_71 > x_73);
        x_85_phi = x_83;
      } else {
        x_84 = (x_71 < x_73);
        x_85_phi = x_84;
      }
      let x_85 : bool = x_85_phi;
      if (x_85) {
        let x_88 : f32 = data[x_70_save];
        let x_89 : f32 = data[x_72_save];
        data[x_70_save] = x_89;
        data[x_72_save] = x_88;
      }

      continuing {
        x_61 = (x_60 + 1);
        x_60_phi = x_61;
      }
    }

    continuing {
      x_54 = (x_53 + 1);
      x_53_phi = x_54;
    }
  }
  let x_91 : f32 = gl_FragCoord.x;
  let x_93 : f32 = x_6.resolution.x;
  if ((x_91 < (x_93 * 0.5))) {
    let x_100 : f32 = data[0];
    let x_103 : f32 = data[5];
    let x_106 : f32 = data[9];
    x_GLF_color = vec4<f32>((x_100 * 0.100000001), (x_103 * 0.100000001), (x_106 * 0.100000001), 1.0);
  } else {
    let x_110 : f32 = data[5];
    let x_113 : f32 = data[9];
    let x_116 : f32 = data[0];
    x_GLF_color = vec4<f32>((x_110 * 0.100000001), (x_113 * 0.100000001), (x_116 * 0.100000001), 1.0);
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
