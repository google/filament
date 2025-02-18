struct buf0 {
  resolution : vec2<f32>,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var indexable : array<vec4<f32>, 16u>;
  var x_76 : vec2<i32>;
  var x_109 : vec2<i32>;
  var x_115 : i32;
  var x_76_phi : vec2<i32>;
  var x_79_phi : i32;
  var x_110_phi : vec2<i32>;
  var x_112_phi : vec2<i32>;
  let x_55 : vec4<f32> = gl_FragCoord;
  let x_58 : vec2<f32> = x_6.resolution;
  let x_59 : vec2<f32> = (vec2<f32>(x_55.x, x_55.y) / x_58);
  let x_62 : i32 = i32((x_59.x * 8.0));
  let x_65 : i32 = i32((x_59.y * 8.0));
  let x_74 : vec2<i32> = vec2<i32>(((((x_62 & 5) | (x_65 & 10)) * 8) + ((x_65 & 5) | (x_62 & 10))), 0);
  x_76_phi = x_74;
  x_79_phi = 0;
  loop {
    var x_90 : vec2<i32>;
    var x_98 : vec2<i32>;
    var x_80 : i32;
    var x_91_phi : vec2<i32>;
    var x_99_phi : vec2<i32>;
    x_76 = x_76_phi;
    let x_79 : i32 = x_79_phi;
    if ((x_79 < 100)) {
    } else {
      break;
    }
    x_91_phi = x_76;
    if ((x_76.x > 0)) {
      x_90 = x_76;
      x_90.y = (x_76.y - 1);
      x_91_phi = x_90;
    }
    let x_91 : vec2<i32> = x_91_phi;
    x_99_phi = x_91;
    if ((x_91.x < 0)) {
      x_98 = x_91;
      x_98.y = (x_91.y + 1);
      x_99_phi = x_98;
    }
    let x_99 : vec2<i32> = x_99_phi;
    var x_77_1 : vec2<i32> = x_99;
    x_77_1.x = (x_99.x + (x_99.y / 2));
    let x_77 : vec2<i32> = x_77_1;

    continuing {
      x_80 = (x_79 + 1);
      x_76_phi = x_77;
      x_79_phi = x_80;
    }
  }
  let x_104 : i32 = x_76.x;
  x_110_phi = x_76;
  if ((x_104 < 0)) {
    x_109 = x_76;
    x_109.x = -(x_104);
    x_110_phi = x_109;
  }
  let x_110 : vec2<i32> = x_110_phi;
  x_112_phi = x_110;
  loop {
    var x_113 : vec2<i32>;
    let x_112 : vec2<i32> = x_112_phi;
    x_115 = x_112.x;
    if ((x_115 > 15)) {
    } else {
      break;
    }

    continuing {
      x_113 = x_112;
      x_113.x = bitcast<i32>((x_115 - bitcast<i32>(16)));
      x_112_phi = x_113;
    }
  }
  indexable = array<vec4<f32>, 16u>(vec4<f32>(0.0, 0.0, 0.0, 1.0), vec4<f32>(0.5, 0.0, 0.0, 1.0), vec4<f32>(0.0, 0.5, 0.0, 1.0), vec4<f32>(0.5, 0.5, 0.0, 1.0), vec4<f32>(0.0, 0.0, 0.5, 1.0), vec4<f32>(0.5, 0.0, 0.5, 1.0), vec4<f32>(0.0, 0.5, 0.5, 1.0), vec4<f32>(0.5, 0.5, 0.5, 1.0), vec4<f32>(0.0, 0.0, 0.0, 1.0), vec4<f32>(1.0, 0.0, 0.0, 1.0), vec4<f32>(0.0, 1.0, 0.0, 1.0), vec4<f32>(1.0, 1.0, 0.0, 1.0), vec4<f32>(0.0, 0.0, 1.0, 1.0), vec4<f32>(1.0, 0.0, 1.0, 1.0), vec4<f32>(0.0, 1.0, 1.0, 1.0), vec4<f32>(1.0, 1.0, 1.0, 1.0));
  let x_120 : vec4<f32> = indexable[x_115];
  x_GLF_color = x_120;
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
