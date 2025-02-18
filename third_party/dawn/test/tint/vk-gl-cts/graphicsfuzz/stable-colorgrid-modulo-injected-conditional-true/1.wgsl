struct buf0 {
  resolution : vec2<f32>,
}

struct buf1 {
  injectionSwitch : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_13 : buf0;

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(1) var<uniform> x_20 : buf1;

fn compute_value_f1_f1_(limit : ptr<function, f32>, thirty_two : ptr<function, f32>) -> f32 {
  var result : f32;
  var i : i32;
  result = -0.5;
  i = 1;
  loop {
    let x_125 : i32 = i;
    if ((x_125 < 800)) {
    } else {
      break;
    }
    let x_128 : i32 = i;
    if (((x_128 % 32) == 0)) {
      let x_134 : f32 = result;
      result = (x_134 + 0.400000006);
    } else {
      let x_136 : i32 = i;
      let x_138 : f32 = *(thirty_two);
      if (((f32(x_136) - (round(x_138) * floor((f32(x_136) / round(x_138))))) <= 0.01)) {
        let x_144 : f32 = result;
        result = (x_144 + 100.0);
      }
    }
    let x_146 : i32 = i;
    let x_148 : f32 = *(limit);
    if ((f32(x_146) >= x_148)) {
      let x_152 : f32 = result;
      return x_152;
    }

    continuing {
      let x_153 : i32 = i;
      i = (x_153 + 1);
    }
  }
  let x_155 : f32 = result;
  return x_155;
}

fn main_1() {
  var c : vec3<f32>;
  var thirty_two_1 : f32;
  var param : f32;
  var param_1 : f32;
  var param_2 : f32;
  var param_3 : f32;
  var i_1 : i32;
  var x_58 : vec3<f32>;
  c = vec3<f32>(7.0, 8.0, 9.0);
  let x_60 : f32 = x_13.resolution.x;
  thirty_two_1 = round((x_60 / 8.0));
  let x_64 : f32 = gl_FragCoord.x;
  param = x_64;
  let x_65 : f32 = thirty_two_1;
  param_1 = x_65;
  let x_66 : f32 = compute_value_f1_f1_(&(param), &(param_1));
  c.x = x_66;
  let x_69 : f32 = gl_FragCoord.y;
  param_2 = x_69;
  let x_70 : f32 = thirty_two_1;
  param_3 = x_70;
  let x_71 : f32 = compute_value_f1_f1_(&(param_2), &(param_3));
  c.y = x_71;
  let x_74 : f32 = c.x;
  let x_76 : f32 = c.y;
  c.z = (x_74 + x_76);
  i_1 = 0;
  loop {
    let x_83 : i32 = i_1;
    if ((x_83 < 3)) {
    } else {
      break;
    }
    let x_86 : i32 = i_1;
    let x_88 : f32 = c[x_86];
    if ((x_88 >= 1.0)) {
      let x_92 : i32 = i_1;
      let x_93 : i32 = i_1;
      let x_95 : f32 = c[x_93];
      let x_96 : i32 = i_1;
      let x_98 : f32 = c[x_96];
      c[x_92] = (x_95 * x_98);
    }

    continuing {
      let x_101 : i32 = i_1;
      i_1 = (x_101 + 1);
    }
  }
  let x_104 : f32 = x_20.injectionSwitch.x;
  let x_106 : f32 = x_20.injectionSwitch.y;
  if ((x_104 < x_106)) {
    let x_111 : vec3<f32> = c;
    x_58 = abs(x_111);
  } else {
    let x_113 : vec3<f32> = c;
    x_58 = x_113;
  }
  let x_114 : vec3<f32> = x_58;
  let x_115 : vec3<f32> = normalize(x_114);
  x_GLF_color = vec4<f32>(x_115.x, x_115.y, x_115.z, 1.0);
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
