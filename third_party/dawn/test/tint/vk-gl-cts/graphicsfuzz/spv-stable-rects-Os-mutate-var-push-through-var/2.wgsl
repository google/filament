struct buf0 {
  resolution : vec2<f32>,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var x_81 : array<vec4<f32>, 8u> = array<vec4<f32>, 8u>(vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0));
  var x_82 : array<vec4<f32>, 8u>;
  var x_83 : array<vec4<f32>, 8u>;
  var x_84 : array<vec4<f32>, 8u>;
  var x_85 : array<vec4<f32>, 16u>;
  var x_95 : vec4<f32>;
  var x_95_phi : vec4<f32>;
  var x_98_phi : i32;
  x_81 = array<vec4<f32>, 8u>(vec4<f32>(4.0, 4.0, 20.0, 4.0), vec4<f32>(4.0, 4.0, 4.0, 20.0), vec4<f32>(4.0, 20.0, 20.0, 4.0), vec4<f32>(20.0, 4.0, 4.0, 8.0), vec4<f32>(8.0, 6.0, 4.0, 2.0), vec4<f32>(2.0, 12.0, 2.0, 4.0), vec4<f32>(16.0, 2.0, 4.0, 4.0), vec4<f32>(12.0, 22.0, 4.0, 4.0));
  let x_86 : array<vec4<f32>, 8u> = x_81;
  let x_87 : vec4<f32> = gl_FragCoord;
  let x_90 : vec2<f32> = x_6.resolution;
  let x_93 : vec2<f32> = floor(((vec2<f32>(x_87.x, x_87.y) / x_90) * 32.0));
  x_95_phi = vec4<f32>(0.5, 0.5, 1.0, 1.0);
  x_98_phi = 0;
  loop {
    var x_142 : vec4<f32>;
    var x_99 : i32;
    var x_127_phi : bool;
    var x_96_phi : vec4<f32>;
    x_95 = x_95_phi;
    let x_98 : i32 = x_98_phi;
    if ((x_98 < 8)) {
    } else {
      break;
    }
    var x_104 : vec4<f32>;
    x_82 = x_86;
    x_104 = x_82[x_98];
    switch(0u) {
      default: {
        let x_107 : f32 = x_93.x;
        let x_108 : f32 = x_104.x;
        if ((x_107 < x_108)) {
          x_127_phi = false;
          break;
        }
        let x_112 : f32 = x_93.y;
        let x_113 : f32 = x_104.y;
        if ((x_112 < x_113)) {
          x_127_phi = false;
          break;
        }
        if ((x_107 > (x_108 + x_104.z))) {
          x_127_phi = false;
          break;
        }
        if ((x_112 > (x_113 + x_104.w))) {
          x_127_phi = false;
          break;
        }
        x_127_phi = true;
      }
    }
    let x_127 : bool = x_127_phi;
    x_96_phi = x_95;
    if (x_127) {
      x_83 = array<vec4<f32>, 8u>(vec4<f32>(4.0, 4.0, 20.0, 4.0), vec4<f32>(4.0, 4.0, 4.0, 20.0), vec4<f32>(4.0, 20.0, 20.0, 4.0), vec4<f32>(20.0, 4.0, 4.0, 8.0), vec4<f32>(8.0, 6.0, 4.0, 2.0), vec4<f32>(2.0, 12.0, 2.0, 4.0), vec4<f32>(16.0, 2.0, 4.0, 4.0), vec4<f32>(12.0, 22.0, 4.0, 4.0));
      let x_131 : f32 = x_83[x_98].x;
      x_84 = array<vec4<f32>, 8u>(vec4<f32>(4.0, 4.0, 20.0, 4.0), vec4<f32>(4.0, 4.0, 4.0, 20.0), vec4<f32>(4.0, 20.0, 20.0, 4.0), vec4<f32>(20.0, 4.0, 4.0, 8.0), vec4<f32>(8.0, 6.0, 4.0, 2.0), vec4<f32>(2.0, 12.0, 2.0, 4.0), vec4<f32>(16.0, 2.0, 4.0, 4.0), vec4<f32>(12.0, 22.0, 4.0, 4.0));
      let x_134 : f32 = x_84[x_98].y;
      x_85 = array<vec4<f32>, 16u>(vec4<f32>(0.0, 0.0, 0.0, 1.0), vec4<f32>(0.5, 0.0, 0.0, 1.0), vec4<f32>(0.0, 0.5, 0.0, 1.0), vec4<f32>(0.5, 0.5, 0.0, 1.0), vec4<f32>(0.0, 0.0, 0.5, 1.0), vec4<f32>(0.5, 0.0, 0.5, 1.0), vec4<f32>(0.0, 0.5, 0.5, 1.0), vec4<f32>(0.5, 0.5, 0.5, 1.0), vec4<f32>(0.0, 0.0, 0.0, 1.0), vec4<f32>(1.0, 0.0, 0.0, 1.0), vec4<f32>(0.0, 1.0, 0.0, 1.0), vec4<f32>(1.0, 1.0, 0.0, 1.0), vec4<f32>(0.0, 0.0, 1.0, 1.0), vec4<f32>(1.0, 0.0, 1.0, 1.0), vec4<f32>(0.0, 1.0, 1.0, 1.0), vec4<f32>(1.0, 1.0, 1.0, 1.0));
      x_142 = x_85[((((i32(x_131) * i32(x_134)) + (x_98 * 9)) + 11) % 16)];
      x_96_phi = x_142;
    }
    let x_96 : vec4<f32> = x_96_phi;

    continuing {
      x_99 = (x_98 + 1);
      x_95_phi = x_96;
      x_98_phi = x_99;
    }
  }
  x_GLF_color = x_95;
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
