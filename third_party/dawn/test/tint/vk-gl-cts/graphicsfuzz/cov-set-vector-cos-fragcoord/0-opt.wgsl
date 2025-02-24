struct buf0 {
  one : f32,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_8 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : vec2<f32>;
  var b : vec3<f32>;
  var x_105 : bool;
  var x_111 : bool;
  var x_106_phi : bool;
  var x_112_phi : bool;
  a = vec2<f32>(1.0, 1.0);
  b = vec3<f32>(0.0, 0.0, 0.0);
  let x_52 : f32 = gl_FragCoord.y;
  if ((i32(x_52) < 40)) {
    b = vec3<f32>(0.100000001, 0.100000001, 0.100000001);
  } else {
    let x_59 : f32 = gl_FragCoord.y;
    if ((i32(x_59) < 60)) {
      b = vec3<f32>(0.200000003, 0.200000003, 0.200000003);
    } else {
      let x_66 : f32 = gl_FragCoord.y;
      if ((x_66 < 80.0)) {
        let x_72 : f32 = a.x;
        let x_74 : f32 = x_8.one;
        b = (cos((vec3<f32>(x_72, x_72, x_72) + vec3<f32>(x_74, x_74, x_74))) + vec3<f32>(0.01, 0.01, 0.01));
      } else {
        let x_82 : f32 = gl_FragCoord.y;
        if ((i32(x_82) < 100)) {
          let x_89 : f32 = x_8.one;
          b = cos(vec3<f32>(x_89, x_89, x_89));
        } else {
          let x_93 : f32 = gl_FragCoord.y;
          if ((i32(x_93) < 500)) {
            b = vec3<f32>(0.540302277, 0.540302277, -0.99996084);
          }
        }
      }
    }
  }
  let x_99 : f32 = b.x;
  let x_100 : bool = (x_99 < 1.019999981);
  x_106_phi = x_100;
  if (x_100) {
    let x_104 : f32 = b.y;
    x_105 = (x_104 < 1.019999981);
    x_106_phi = x_105;
  }
  let x_106 : bool = x_106_phi;
  x_112_phi = x_106;
  if (x_106) {
    let x_110 : f32 = b.z;
    x_111 = (x_110 < 1.019999981);
    x_112_phi = x_111;
  }
  let x_112 : bool = x_112_phi;
  if (x_112) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
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
