struct buf0 {
  resolution : vec2<f32>,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var indexable : array<vec4<f32>, 16u>;
  var x_77 : vec2<i32>;
  var x_110 : vec2<i32>;
  var x_116 : i32;
  var x_77_phi : vec2<i32>;
  var x_80_phi : i32;
  var x_111_phi : vec2<i32>;
  var x_113_phi : vec2<i32>;
  let x_56 : vec4<f32> = gl_FragCoord;
  let x_59 : vec2<f32> = x_6.resolution;
  let x_60 : vec2<f32> = (vec2<f32>(x_56.x, x_56.y) / x_59);
  let x_63 : i32 = i32((x_60.x * 8.0));
  let x_66 : i32 = i32((x_60.y * 8.0));
  let x_75 : vec2<i32> = vec2<i32>(((((x_63 & 5) | (x_66 & 10)) * 8) + ((x_66 & 5) | (x_63 & 10))), 0);
  x_77_phi = x_75;
  x_80_phi = 0;
  loop {
    var x_91 : vec2<i32>;
    var x_99 : vec2<i32>;
    var x_81 : i32;
    var x_92_phi : vec2<i32>;
    var x_100_phi : vec2<i32>;
    x_77 = x_77_phi;
    let x_80 : i32 = x_80_phi;
    if ((x_80 < 100)) {
    } else {
      break;
    }
    x_92_phi = x_77;
    if ((x_77.x > 0)) {
      x_91 = x_77;
      x_91.y = (x_77.y - 1);
      x_92_phi = x_91;
    }
    let x_92 : vec2<i32> = x_92_phi;
    x_100_phi = x_92;
    if ((x_92.x < 0)) {
      x_99 = x_92;
      x_99.y = (x_92.y + 1);
      x_100_phi = x_99;
    }
    let x_100 : vec2<i32> = x_100_phi;
    var x_78_1 : vec2<i32> = x_100;
    x_78_1.x = (x_100.x + (x_100.y / 2));
    let x_78 : vec2<i32> = x_78_1;

    continuing {
      x_81 = (x_80 + 1);
      x_77_phi = x_78;
      x_80_phi = x_81;
    }
  }
  let x_105 : i32 = x_77.x;
  x_111_phi = x_77;
  if ((x_105 < 0)) {
    x_110 = vec2<i32>();
    x_110.x = -(x_105);
    x_111_phi = x_110;
  }
  let x_111 : vec2<i32> = x_111_phi;
  x_113_phi = x_111;
  loop {
    var x_114 : vec2<i32>;
    let x_113 : vec2<i32> = x_113_phi;
    x_116 = x_113.x;
    if ((x_116 > 15)) {
    } else {
      break;
    }

    continuing {
      x_114 = vec2<i32>();
      x_114.x = bitcast<i32>((x_116 - bitcast<i32>(16)));
      x_113_phi = x_114;
    }
  }
  indexable = array<vec4<f32>, 16u>(vec4<f32>(0.0, 0.0, 0.0, 1.0), vec4<f32>(0.5, 0.0, 0.0, 1.0), vec4<f32>(0.0, 0.5, 0.0, 1.0), vec4<f32>(0.5, 0.5, 0.0, 1.0), vec4<f32>(0.0, 0.0, 0.5, 1.0), vec4<f32>(0.5, 0.0, 0.5, 1.0), vec4<f32>(0.0, 0.5, 0.5, 1.0), vec4<f32>(0.5, 0.5, 0.5, 1.0), vec4<f32>(0.0, 0.0, 0.0, 1.0), vec4<f32>(1.0, 0.0, 0.0, 1.0), vec4<f32>(0.0, 1.0, 0.0, 1.0), vec4<f32>(1.0, 1.0, 0.0, 1.0), vec4<f32>(0.0, 0.0, 1.0, 1.0), vec4<f32>(1.0, 0.0, 1.0, 1.0), vec4<f32>(0.0, 1.0, 1.0, 1.0), vec4<f32>(1.0, 1.0, 1.0, 1.0));
  let x_121 : vec4<f32> = indexable[x_116];
  x_GLF_color = x_121;
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
