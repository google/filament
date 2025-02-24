struct buf0 {
  injectionSwitch : vec2<f32>,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_10 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn func_() -> f32 {
  var alwaysFalse : bool;
  var value : vec4<f32>;
  var a : vec2<f32>;
  var i : i32;
  var x_121 : bool;
  var x_122_phi : bool;
  let x_71 : f32 = gl_FragCoord.x;
  alwaysFalse = (x_71 < -1.0);
  let x_73 : bool = alwaysFalse;
  if (x_73) {
    let x_76 : vec2<f32> = a;
    let x_77 : vec4<f32> = value;
    value = vec4<f32>(x_76.x, x_76.y, x_77.z, x_77.w);
  }
  let x_79 : bool = alwaysFalse;
  if (!(x_79)) {
    let x_84 : vec2<f32> = x_10.injectionSwitch;
    let x_85 : vec4<f32> = value;
    value = vec4<f32>(x_84.x, x_84.y, x_85.z, x_85.w);
  }
  let x_87 : vec4<f32> = gl_FragCoord;
  let x_89 : vec4<f32> = value;
  let x_93 : vec4<f32> = value;
  let x_95 : vec2<f32> = (((vec2<f32>(x_87.x, x_87.y) * vec2<f32>(x_89.x, x_89.y)) * vec2<f32>(2.0, 2.0)) + vec2<f32>(x_93.x, x_93.y));
  let x_96 : vec4<f32> = value;
  value = vec4<f32>(x_96.x, x_96.y, x_95.x, x_95.y);
  i = 0;
  loop {
    let x_102 : i32 = i;
    let x_104 : f32 = x_10.injectionSwitch.y;
    if ((x_102 < (i32(x_104) + 1))) {
    } else {
      break;
    }
    let x_109 : i32 = i;
    value.x = f32(x_109);

    continuing {
      let x_112 : i32 = i;
      i = (x_112 + 1);
    }
  }
  let x_115 : f32 = value.x;
  let x_116 : bool = (x_115 == 1.0);
  x_122_phi = x_116;
  if (x_116) {
    let x_120 : f32 = value.y;
    x_121 = (x_120 == 1.0);
    x_122_phi = x_121;
  }
  let x_122 : bool = x_122_phi;
  if (x_122) {
    return 1.0;
  } else {
    return 0.0;
  }
}

fn main_1() {
  var count : i32;
  var i_1 : i32;
  count = 0;
  i_1 = 0;
  loop {
    let x_51 : i32 = i_1;
    let x_53 : f32 = x_10.injectionSwitch.y;
    if ((x_51 < (i32(x_53) + 1))) {
    } else {
      break;
    }
    let x_58 : f32 = func_();
    let x_60 : i32 = count;
    count = (x_60 + i32(x_58));

    continuing {
      let x_62 : i32 = i_1;
      i_1 = (x_62 + 1);
    }
  }
  let x_64 : i32 = count;
  if ((x_64 == 2)) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  } else {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 1.0);
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
