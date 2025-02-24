struct buf0 {
  resolution : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var c : vec3<f32>;
  var x_53 : f32;
  var x_57 : f32;
  var x_58 : f32;
  var x_83 : f32;
  var x_84 : f32;
  var x_124 : f32;
  var x_125 : f32;
  var x_57_phi : f32;
  var x_60_phi : i32;
  var x_83_phi : f32;
  var x_84_phi : f32;
  var x_85_phi : bool;
  var x_87_phi : f32;
  var x_128_phi : f32;
  var x_135_phi : i32;
  c = vec3<f32>(7.0, 8.0, 9.0);
  let x_47 : f32 = x_7.resolution.x;
  let x_48 : vec2<f32> = vec2<f32>(1.0, x_47);
  let x_50 : f32 = round((x_47 * 0.125));
  let x_51 : vec2<f32> = vec2<f32>(0.0, -0.5);
  x_53 = gl_FragCoord.x;
  switch(0u) {
    default: {
      x_57_phi = -0.5;
      x_60_phi = 1;
      loop {
        var x_70 : f32;
        var x_78 : f32;
        var x_61 : i32;
        var x_58_phi : f32;
        x_57 = x_57_phi;
        let x_60 : i32 = x_60_phi;
        x_83_phi = 0.0;
        x_84_phi = x_57;
        x_85_phi = false;
        if ((x_60 < 800)) {
        } else {
          break;
        }
        var x_77 : f32;
        var x_78_phi : f32;
        if (((x_60 % 32) == 0)) {
          x_70 = (x_57 + 0.400000006);
          x_58_phi = x_70;
        } else {
          x_78_phi = x_57;
          if (((f32(x_60) - (round(x_50) * floor((f32(x_60) / round(x_50))))) <= 0.01)) {
            x_77 = (x_57 + 100.0);
            x_78_phi = x_77;
          }
          x_78 = x_78_phi;
          x_58_phi = x_78;
        }
        x_58 = x_58_phi;
        if ((f32(x_60) >= x_53)) {
          x_83_phi = x_58;
          x_84_phi = x_58;
          x_85_phi = true;
          break;
        }

        continuing {
          x_61 = (x_60 + 1);
          x_57_phi = x_58;
          x_60_phi = x_61;
        }
      }
      x_83 = x_83_phi;
      x_84 = x_84_phi;
      let x_85 : bool = x_85_phi;
      x_87_phi = x_83;
      if (x_85) {
        break;
      }
      x_87_phi = x_84;
    }
  }
  var x_92 : f32;
  var x_98 : f32;
  var x_99 : f32;
  var x_98_phi : f32;
  var x_101_phi : i32;
  var x_124_phi : f32;
  var x_125_phi : f32;
  var x_126_phi : bool;
  let x_87 : f32 = x_87_phi;
  let x_89 : vec4<f32> = vec4<f32>(x_84, 0.400000006, x_83, 0.400000006);
  c.x = x_87;
  x_92 = gl_FragCoord.y;
  switch(0u) {
    default: {
      let x_95 : vec4<f32> = vec4<f32>(x_51, 0.0, x_57);
      let x_96 : f32 = vec3<f32>(x_48, -0.5).z;
      x_98_phi = x_96;
      x_101_phi = 1;
      loop {
        var x_111 : f32;
        var x_119 : f32;
        var x_102 : i32;
        var x_99_phi : f32;
        x_98 = x_98_phi;
        let x_101 : i32 = x_101_phi;
        x_124_phi = 0.0;
        x_125_phi = x_98;
        x_126_phi = false;
        if ((x_101 < 800)) {
        } else {
          break;
        }
        var x_118 : f32;
        var x_119_phi : f32;
        if (((x_101 % 32) == 0)) {
          x_111 = (x_98 + 0.400000006);
          x_99_phi = x_111;
        } else {
          x_119_phi = x_98;
          if (((f32(x_101) - (round(x_50) * floor((f32(x_101) / round(x_50))))) <= 0.01)) {
            x_118 = (x_98 + 100.0);
            x_119_phi = x_118;
          }
          x_119 = x_119_phi;
          x_99_phi = x_119;
        }
        x_99 = x_99_phi;
        if ((f32(x_101) >= x_92)) {
          x_124_phi = x_99;
          x_125_phi = x_99;
          x_126_phi = true;
          break;
        }

        continuing {
          x_102 = (x_101 + 1);
          x_98_phi = x_99;
          x_101_phi = x_102;
        }
      }
      x_124 = x_124_phi;
      x_125 = x_125_phi;
      let x_126 : bool = x_126_phi;
      x_128_phi = x_124;
      if (x_126) {
        break;
      }
      x_128_phi = x_125;
    }
  }
  let x_128 : f32 = x_128_phi;
  c.y = x_128;
  let x_130 : f32 = c.x;
  let x_131 : f32 = c.y;
  c.z = (x_130 + x_131);
  x_135_phi = 0;
  loop {
    var x_136 : i32;
    let x_135 : i32 = x_135_phi;
    if ((x_135 < 3)) {
    } else {
      break;
    }
    let x_142 : f32 = c[x_135];
    if ((x_142 >= 1.0)) {
      let x_146 : f32 = c[x_135];
      let x_147 : f32 = c[x_135];
      c[x_135] = (x_146 * x_147);
    }

    continuing {
      x_136 = (x_135 + 1);
      x_135_phi = x_136;
    }
  }
  let x_149 : vec3<f32> = c;
  let x_151 : vec3<f32> = normalize(abs(x_149));
  x_GLF_color = vec4<f32>(x_151.x, x_151.y, x_151.z, 1.0);
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
