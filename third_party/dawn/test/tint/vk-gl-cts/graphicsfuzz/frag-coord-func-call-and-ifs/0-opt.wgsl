struct buf0 {
  injectionSwitch : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_9 : buf0;

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var x_43 : f32;
  var x_44 : f32;
  var x_45 : f32;
  var x_46 : i32;
  var zero : i32;
  var param : vec2<f32>;
  var temp : vec2<f32>;
  let x_47 : vec4<f32> = gl_FragCoord;
  param = vec2<f32>(x_47.x, x_47.y);
  loop {
    let x_54 : f32 = param.y;
    if ((x_54 < 50.0)) {
      let x_60 : f32 = x_9.injectionSwitch.y;
      x_44 = x_60;
    } else {
      x_44 = 0.0;
    }
    let x_61 : f32 = x_44;
    x_43 = x_61;
    let x_63 : f32 = gl_FragCoord.y;
    let x_65 : f32 = select(0.0, 1.0, (x_63 < 50.0));
    x_45 = x_65;
    if (((x_61 - x_65) < 1.0)) {
      x_46 = 0;
      break;
    }
    x_46 = 1;
    break;

    continuing {
      break if !(false);
    }
  }
  let x_70 : i32 = x_46;
  zero = x_70;
  if ((x_70 == 1)) {
    return;
  }
  x_GLF_color = vec4<f32>(0.0, 1.0, 1.0, 1.0);
  let x_75 : f32 = gl_FragCoord.x;
  let x_77 : f32 = x_9.injectionSwitch.x;
  if ((x_75 >= x_77)) {
    let x_82 : f32 = gl_FragCoord.y;
    if ((x_82 >= 0.0)) {
      let x_87 : f32 = x_9.injectionSwitch.y;
      x_GLF_color.x = x_87;
    }
  }
  let x_90 : f32 = gl_FragCoord.y;
  if ((x_90 >= 0.0)) {
    let x_95 : f32 = x_9.injectionSwitch.x;
    x_GLF_color.y = x_95;
  }
  let x_97 : vec4<f32> = gl_FragCoord;
  let x_98 : vec2<f32> = vec2<f32>(x_97.x, x_97.y);
  let x_101 : vec2<f32> = vec2<f32>(x_98.x, x_98.y);
  temp = x_101;
  if ((x_101.y >= 0.0)) {
    let x_107 : f32 = x_9.injectionSwitch.x;
    x_GLF_color.z = x_107;
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

fn alwaysZero_vf2_(coord : ptr<function, vec2<f32>>) -> i32 {
  var a : f32;
  var x_110 : f32;
  var b : f32;
  let x_112 : f32 = (*(coord)).y;
  if ((x_112 < 50.0)) {
    let x_118 : f32 = x_9.injectionSwitch.y;
    x_110 = x_118;
  } else {
    x_110 = 0.0;
  }
  let x_119 : f32 = x_110;
  a = x_119;
  let x_121 : f32 = gl_FragCoord.y;
  let x_123 : f32 = select(0.0, 1.0, (x_121 < 50.0));
  b = x_123;
  if (((x_119 - x_123) < 1.0)) {
    return 0;
  }
  return 1;
}
