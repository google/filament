struct buf0 {
  resolution : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var c : vec3<f32>;
  var x_51 : f32;
  var x_55 : f32;
  var x_56 : f32;
  var x_81 : f32;
  var x_82 : f32;
  var x_118 : f32;
  var x_119 : f32;
  var x_55_phi : f32;
  var x_58_phi : i32;
  var x_81_phi : f32;
  var x_82_phi : f32;
  var x_83_phi : bool;
  var x_85_phi : f32;
  var x_122_phi : f32;
  var x_129_phi : i32;
  c = vec3<f32>(7.0, 8.0, 9.0);
  let x_47 : f32 = x_7.resolution.x;
  let x_49 : f32 = round((x_47 * 0.125));
  x_51 = gl_FragCoord.x;
  switch(0u) {
    default: {
      x_55_phi = -0.5;
      x_58_phi = 1;
      loop {
        var x_68 : f32;
        var x_76 : f32;
        var x_59 : i32;
        var x_56_phi : f32;
        x_55 = x_55_phi;
        let x_58 : i32 = x_58_phi;
        x_81_phi = 0.0;
        x_82_phi = x_55;
        x_83_phi = false;
        if ((x_58 < 800)) {
        } else {
          break;
        }
        var x_75 : f32;
        var x_76_phi : f32;
        if (((x_58 % 32) == 0)) {
          x_68 = (x_55 + 0.400000006);
          x_56_phi = x_68;
        } else {
          x_76_phi = x_55;
          if (((f32(x_58) - (round(x_49) * floor((f32(x_58) / round(x_49))))) <= 0.01)) {
            x_75 = (x_55 + 100.0);
            x_76_phi = x_75;
          }
          x_76 = x_76_phi;
          x_56_phi = x_76;
        }
        x_56 = x_56_phi;
        if ((f32(x_58) >= x_51)) {
          x_81_phi = x_56;
          x_82_phi = x_56;
          x_83_phi = true;
          break;
        }

        continuing {
          x_59 = (x_58 + 1);
          x_55_phi = x_56;
          x_58_phi = x_59;
        }
      }
      x_81 = x_81_phi;
      x_82 = x_82_phi;
      let x_83 : bool = x_83_phi;
      x_85_phi = x_81;
      if (x_83) {
        break;
      }
      x_85_phi = x_82;
    }
  }
  var x_88 : f32;
  var x_92 : f32;
  var x_93 : f32;
  var x_92_phi : f32;
  var x_95_phi : i32;
  var x_118_phi : f32;
  var x_119_phi : f32;
  var x_120_phi : bool;
  let x_85 : f32 = x_85_phi;
  c.x = x_85;
  x_88 = gl_FragCoord.y;
  switch(0u) {
    default: {
      x_92_phi = -0.5;
      x_95_phi = 1;
      loop {
        var x_113 : f32;
        var x_112 : f32;
        var x_96 : i32;
        var x_93_phi : f32;
        x_92 = x_92_phi;
        let x_95 : i32 = x_95_phi;
        x_118_phi = 0.0;
        x_119_phi = x_92;
        x_120_phi = false;
        if ((x_95 < 800)) {
        } else {
          break;
        }
        var x_111 : f32;
        var x_112_phi : f32;
        if (((x_95 % 32) == 0)) {
          x_113 = (x_92 + 0.400000006);
          x_93_phi = x_113;
        } else {
          x_112_phi = x_92;
          if (((f32(x_95) - (round(x_49) * floor((f32(x_95) / round(x_49))))) <= 0.01)) {
            x_111 = (x_92 + 100.0);
            x_112_phi = x_111;
          }
          x_112 = x_112_phi;
          x_93_phi = x_112;
        }
        x_93 = x_93_phi;
        if ((f32(x_95) >= x_88)) {
          x_118_phi = x_93;
          x_119_phi = x_93;
          x_120_phi = true;
          break;
        }

        continuing {
          x_96 = (x_95 + 1);
          x_92_phi = x_93;
          x_95_phi = x_96;
        }
      }
      x_118 = x_118_phi;
      x_119 = x_119_phi;
      let x_120 : bool = x_120_phi;
      x_122_phi = x_118;
      if (x_120) {
        break;
      }
      x_122_phi = x_119;
    }
  }
  let x_122 : f32 = x_122_phi;
  c.y = x_122;
  let x_124 : f32 = c.x;
  let x_125 : f32 = c.y;
  c.z = (x_124 + x_125);
  x_129_phi = 0;
  loop {
    var x_130 : i32;
    let x_129 : i32 = x_129_phi;
    if ((x_129 < 3)) {
    } else {
      break;
    }
    let x_136 : f32 = c[x_129];
    if ((x_136 >= 1.0)) {
      let x_140 : f32 = c[x_129];
      let x_141 : f32 = c[x_129];
      c[x_129] = (x_140 * x_141);
    }

    continuing {
      x_130 = (x_129 + 1);
      x_129_phi = x_130;
    }
  }
  let x_143 : vec3<f32> = c;
  let x_145 : vec3<f32> = normalize(abs(x_143));
  x_GLF_color = vec4<f32>(x_145.x, x_145.y, x_145.z, 1.0);
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
