struct buf0 {
  resolution : vec2<f32>,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var x_77 : array<vec4<f32>, 8u>;
  var x_78 : array<vec4<f32>, 8u>;
  var x_79 : array<vec4<f32>, 8u>;
  var x_80 : array<vec4<f32>, 16u>;
  var x_89 : vec4<f32>;
  var x_89_phi : vec4<f32>;
  var x_92_phi : i32;
  let x_81 : vec4<f32> = gl_FragCoord;
  let x_84 : vec2<f32> = x_6.resolution;
  let x_87 : vec2<f32> = floor(((vec2<f32>(x_81.x, x_81.y) / x_84) * 32.0));
  x_89_phi = vec4<f32>(0.5, 0.5, 1.0, 1.0);
  x_92_phi = 0;
  loop {
    var x_136 : vec4<f32>;
    var x_93 : i32;
    var x_121_phi : bool;
    var x_90_phi : vec4<f32>;
    x_89 = x_89_phi;
    let x_92 : i32 = x_92_phi;
    if ((x_92 < 8)) {
    } else {
      break;
    }
    var x_98 : vec4<f32>;
    x_77 = array<vec4<f32>, 8u>(vec4<f32>(4.0, 4.0, 20.0, 4.0), vec4<f32>(4.0, 4.0, 4.0, 20.0), vec4<f32>(4.0, 20.0, 20.0, 4.0), vec4<f32>(20.0, 4.0, 4.0, 8.0), vec4<f32>(8.0, 6.0, 4.0, 2.0), vec4<f32>(2.0, 12.0, 2.0, 4.0), vec4<f32>(16.0, 2.0, 4.0, 4.0), vec4<f32>(12.0, 22.0, 4.0, 4.0));
    x_98 = x_77[x_92];
    switch(0u) {
      default: {
        let x_101 : f32 = x_87.x;
        let x_102 : f32 = x_98.x;
        if ((x_101 < x_102)) {
          x_121_phi = false;
          break;
        }
        let x_106 : f32 = x_87.y;
        let x_107 : f32 = x_98.y;
        if ((x_106 < x_107)) {
          x_121_phi = false;
          break;
        }
        if ((x_101 > (x_102 + x_98.z))) {
          x_121_phi = false;
          break;
        }
        if ((x_106 > (x_107 + x_98.w))) {
          x_121_phi = false;
          break;
        }
        x_121_phi = true;
      }
    }
    let x_121 : bool = x_121_phi;
    x_90_phi = x_89;
    if (x_121) {
      x_78 = array<vec4<f32>, 8u>(vec4<f32>(4.0, 4.0, 20.0, 4.0), vec4<f32>(4.0, 4.0, 4.0, 20.0), vec4<f32>(4.0, 20.0, 20.0, 4.0), vec4<f32>(20.0, 4.0, 4.0, 8.0), vec4<f32>(8.0, 6.0, 4.0, 2.0), vec4<f32>(2.0, 12.0, 2.0, 4.0), vec4<f32>(16.0, 2.0, 4.0, 4.0), vec4<f32>(12.0, 22.0, 4.0, 4.0));
      let x_125 : f32 = x_78[x_92].x;
      x_79 = array<vec4<f32>, 8u>(vec4<f32>(4.0, 4.0, 20.0, 4.0), vec4<f32>(4.0, 4.0, 4.0, 20.0), vec4<f32>(4.0, 20.0, 20.0, 4.0), vec4<f32>(20.0, 4.0, 4.0, 8.0), vec4<f32>(8.0, 6.0, 4.0, 2.0), vec4<f32>(2.0, 12.0, 2.0, 4.0), vec4<f32>(16.0, 2.0, 4.0, 4.0), vec4<f32>(12.0, 22.0, 4.0, 4.0));
      let x_128 : f32 = x_79[x_92].y;
      x_80 = array<vec4<f32>, 16u>(vec4<f32>(0.0, 0.0, 0.0, 1.0), vec4<f32>(0.5, 0.0, 0.0, 1.0), vec4<f32>(0.0, 0.5, 0.0, 1.0), vec4<f32>(0.5, 0.5, 0.0, 1.0), vec4<f32>(0.0, 0.0, 0.5, 1.0), vec4<f32>(0.5, 0.0, 0.5, 1.0), vec4<f32>(0.0, 0.5, 0.5, 1.0), vec4<f32>(0.5, 0.5, 0.5, 1.0), vec4<f32>(0.0, 0.0, 0.0, 1.0), vec4<f32>(1.0, 0.0, 0.0, 1.0), vec4<f32>(0.0, 1.0, 0.0, 1.0), vec4<f32>(1.0, 1.0, 0.0, 1.0), vec4<f32>(0.0, 0.0, 1.0, 1.0), vec4<f32>(1.0, 0.0, 1.0, 1.0), vec4<f32>(0.0, 1.0, 1.0, 1.0), vec4<f32>(1.0, 1.0, 1.0, 1.0));
      x_136 = x_80[((((i32(x_125) * i32(x_128)) + (x_92 * 9)) + 11) % 16)];
      x_90_phi = x_136;
    }
    let x_90 : vec4<f32> = x_90_phi;

    continuing {
      x_93 = (x_92 + 1);
      x_89_phi = x_90;
      x_92_phi = x_93;
    }
  }
  x_GLF_color = x_89;
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
