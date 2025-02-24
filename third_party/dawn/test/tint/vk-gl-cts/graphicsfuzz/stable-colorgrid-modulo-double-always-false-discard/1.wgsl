struct buf1 {
  injectionSwitch : vec2<f32>,
}

struct buf0 {
  resolution : vec2<f32>,
}

@group(0) @binding(1) var<uniform> x_6 : buf1;

@group(0) @binding(0) var<uniform> x_9 : buf0;

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var c : vec3<f32>;
  var x_54 : f32;
  var x_58 : f32;
  var x_59 : f32;
  var x_91 : f32;
  var x_92 : f32;
  var x_135 : f32;
  var x_136 : f32;
  var x_58_phi : f32;
  var x_61_phi : i32;
  var x_91_phi : f32;
  var x_92_phi : f32;
  var x_93_phi : bool;
  var x_95_phi : f32;
  var x_139_phi : f32;
  var x_146_phi : i32;
  c = vec3<f32>(7.0, 8.0, 9.0);
  let x_50 : f32 = x_9.resolution.x;
  let x_52 : f32 = round((x_50 * 0.125));
  x_54 = gl_FragCoord.x;
  switch(0u) {
    default: {
      x_58_phi = -0.5;
      x_61_phi = 1;
      loop {
        var x_71 : f32;
        var x_79 : f32;
        var x_62 : i32;
        var x_59_phi : f32;
        x_58 = x_58_phi;
        let x_61 : i32 = x_61_phi;
        x_91_phi = 0.0;
        x_92_phi = x_58;
        x_93_phi = false;
        if ((x_61 < 800)) {
        } else {
          break;
        }
        var x_78 : f32;
        var x_79_phi : f32;
        if (((x_61 % 32) == 0)) {
          x_71 = (x_58 + 0.400000006);
          x_59_phi = x_71;
        } else {
          x_79_phi = x_58;
          if (((f32(x_61) - (round(x_52) * floor((f32(x_61) / round(x_52))))) <= 0.01)) {
            x_78 = (x_58 + 100.0);
            x_79_phi = x_78;
          }
          x_79 = x_79_phi;
          let x_81 : f32 = x_6.injectionSwitch.x;
          let x_83 : f32 = x_6.injectionSwitch.y;
          if ((x_81 > x_83)) {
            discard;
          }
          x_59_phi = x_79;
        }
        x_59 = x_59_phi;
        if ((f32(x_61) >= x_54)) {
          x_91_phi = x_59;
          x_92_phi = x_59;
          x_93_phi = true;
          break;
        }

        continuing {
          x_62 = (x_61 + 1);
          x_58_phi = x_59;
          x_61_phi = x_62;
        }
      }
      x_91 = x_91_phi;
      x_92 = x_92_phi;
      let x_93 : bool = x_93_phi;
      x_95_phi = x_91;
      if (x_93) {
        break;
      }
      x_95_phi = x_92;
    }
  }
  var x_98 : f32;
  var x_102 : f32;
  var x_103 : f32;
  var x_102_phi : f32;
  var x_105_phi : i32;
  var x_135_phi : f32;
  var x_136_phi : f32;
  var x_137_phi : bool;
  let x_95 : f32 = x_95_phi;
  c.x = x_95;
  x_98 = gl_FragCoord.y;
  switch(0u) {
    default: {
      x_102_phi = -0.5;
      x_105_phi = 1;
      loop {
        var x_115 : f32;
        var x_123 : f32;
        var x_106 : i32;
        var x_103_phi : f32;
        x_102 = x_102_phi;
        let x_105 : i32 = x_105_phi;
        x_135_phi = 0.0;
        x_136_phi = x_102;
        x_137_phi = false;
        if ((x_105 < 800)) {
        } else {
          break;
        }
        var x_122 : f32;
        var x_123_phi : f32;
        if (((x_105 % 32) == 0)) {
          x_115 = (x_102 + 0.400000006);
          x_103_phi = x_115;
        } else {
          x_123_phi = x_102;
          if (((f32(x_105) - (round(x_52) * floor((f32(x_105) / round(x_52))))) <= 0.01)) {
            x_122 = (x_102 + 100.0);
            x_123_phi = x_122;
          }
          x_123 = x_123_phi;
          let x_125 : f32 = x_6.injectionSwitch.x;
          let x_127 : f32 = x_6.injectionSwitch.y;
          if ((x_125 > x_127)) {
            discard;
          }
          x_103_phi = x_123;
        }
        x_103 = x_103_phi;
        if ((f32(x_105) >= x_98)) {
          x_135_phi = x_103;
          x_136_phi = x_103;
          x_137_phi = true;
          break;
        }

        continuing {
          x_106 = (x_105 + 1);
          x_102_phi = x_103;
          x_105_phi = x_106;
        }
      }
      x_135 = x_135_phi;
      x_136 = x_136_phi;
      let x_137 : bool = x_137_phi;
      x_139_phi = x_135;
      if (x_137) {
        break;
      }
      x_139_phi = x_136;
    }
  }
  let x_139 : f32 = x_139_phi;
  c.y = x_139;
  let x_141 : f32 = c.x;
  let x_142 : f32 = c.y;
  c.z = (x_141 + x_142);
  x_146_phi = 0;
  loop {
    var x_147 : i32;
    let x_146 : i32 = x_146_phi;
    if ((x_146 < 3)) {
    } else {
      break;
    }
    let x_153 : f32 = c[x_146];
    if ((x_153 >= 1.0)) {
      let x_157 : f32 = c[x_146];
      let x_158 : f32 = c[x_146];
      c[x_146] = (x_157 * x_158);
      let x_161 : f32 = x_6.injectionSwitch.x;
      let x_163 : f32 = x_6.injectionSwitch.y;
      if ((x_161 > x_163)) {
        discard;
      }
    }

    continuing {
      x_147 = (x_146 + 1);
      x_146_phi = x_147;
    }
  }
  let x_167 : vec3<f32> = c;
  let x_169 : vec3<f32> = normalize(abs(x_167));
  x_GLF_color = vec4<f32>(x_169.x, x_169.y, x_169.z, 1.0);
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
