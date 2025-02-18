struct buf0 {
  one : f32,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var m44 : mat4x4<f32>;
  var x_10_phi : i32;
  m44 = mat4x4<f32>(vec4<f32>(1.0, 2.0, 3.0, 4.0), vec4<f32>(5.0, 6.0, 7.0, 8.0), vec4<f32>(9.0, 10.0, 11.0, 12.0), vec4<f32>(13.0, 14.0, 15.0, 16.0));
  x_10_phi = 0;
  loop {
    var x_9 : i32;
    var x_11_phi : i32;
    let x_10 : i32 = x_10_phi;
    if ((x_10 < 4)) {
    } else {
      break;
    }
    let x_63 : f32 = gl_FragCoord.y;
    if ((x_63 < 0.0)) {
      break;
    }
    x_11_phi = 0;
    loop {
      var x_8 : i32;
      let x_11 : i32 = x_11_phi;
      if ((x_11 < 4)) {
      } else {
        break;
      }

      continuing {
        let x_72 : f32 = x_7.one;
        let x_74 : f32 = m44[x_10][x_11];
        m44[x_10][x_11] = (x_74 + x_72);
        x_8 = (x_11 + 1);
        x_11_phi = x_8;
      }
    }

    continuing {
      x_9 = (x_10 + 1);
      x_10_phi = x_9;
    }
  }
  let x_77 : f32 = m44[1].y;
  var x_79_1 : vec4<f32> = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  x_79_1.x = (x_77 - 6.0);
  let x_79 : vec4<f32> = x_79_1;
  let x_81 : f32 = m44[2].z;
  var x_83_1 : vec4<f32> = x_79;
  x_83_1.w = (x_81 - 11.0);
  let x_83 : vec4<f32> = x_83_1;
  x_GLF_color = x_83;
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
