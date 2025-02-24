struct buf0 {
  resolution : vec2<f32>,
}

struct buf1 {
  injectionSwitch : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_10 : buf0;

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(1) var<uniform> x_16 : buf1;

var<private> x_GLF_color : vec4<f32>;

fn compute_value_f1_f1_(limit : ptr<function, f32>, thirty_two : ptr<function, f32>) -> f32 {
  var x_104 : f32;
  var x_104_phi : f32;
  var x_107_phi : i32;
  x_104_phi = -0.5;
  x_107_phi = 1;
  loop {
    var x_126 : f32;
    var x_125 : f32;
    var x_108 : i32;
    var x_105_phi : f32;
    x_104 = x_104_phi;
    let x_107 : i32 = x_107_phi;
    if ((x_107 < 800)) {
    } else {
      break;
    }
    var x_124 : f32;
    var x_125_phi : f32;
    if (((x_107 % 32) == 0)) {
      x_126 = (x_104 + 0.400000006);
      x_105_phi = x_126;
    } else {
      let x_118 : f32 = *(thirty_two);
      x_125_phi = x_104;
      if (((f32(x_107) - (round(x_118) * floor((f32(x_107) / round(x_118))))) <= 0.01)) {
        x_124 = (x_104 + 100.0);
        x_125_phi = x_124;
      }
      x_125 = x_125_phi;
      x_105_phi = x_125;
    }
    var x_105 : f32;
    x_105 = x_105_phi;
    let x_128 : f32 = *(limit);
    if ((f32(x_107) >= x_128)) {
      return x_105;
    }

    continuing {
      x_108 = (x_107 + 1);
      x_104_phi = x_105;
      x_107_phi = x_108;
    }
  }
  return x_104;
}

fn main_1() {
  var c : vec3<f32>;
  var param : f32;
  var param_1 : f32;
  var param_2 : f32;
  var param_3 : f32;
  var x_54 : vec3<f32>;
  var x_74_phi : i32;
  c = vec3<f32>(7.0, 8.0, 9.0);
  let x_56 : f32 = x_10.resolution.x;
  let x_58 : f32 = round((x_56 * 0.125));
  let x_60 : f32 = gl_FragCoord.x;
  param = x_60;
  param_1 = x_58;
  let x_61 : f32 = compute_value_f1_f1_(&(param), &(param_1));
  c.x = x_61;
  let x_64 : f32 = gl_FragCoord.y;
  param_2 = x_64;
  param_3 = x_58;
  let x_65 : f32 = compute_value_f1_f1_(&(param_2), &(param_3));
  c.y = x_65;
  let x_67 : f32 = c.x;
  let x_68 : vec3<f32> = c;
  x_54 = x_68;
  let x_70 : f32 = x_54.y;
  c.z = (x_67 + x_70);
  x_74_phi = 0;
  loop {
    var x_75 : i32;
    let x_74 : i32 = x_74_phi;
    if ((x_74 < 3)) {
    } else {
      break;
    }
    let x_81 : f32 = c[x_74];
    if ((x_81 >= 1.0)) {
      let x_86 : f32 = x_16.injectionSwitch.x;
      let x_88 : f32 = x_16.injectionSwitch.y;
      if ((x_86 > x_88)) {
        discard;
      }
      let x_92 : f32 = c[x_74];
      let x_93 : f32 = c[x_74];
      c[x_74] = (x_92 * x_93);
    }

    continuing {
      x_75 = (x_74 + 1);
      x_74_phi = x_75;
    }
  }
  let x_95 : vec3<f32> = c;
  let x_97 : vec3<f32> = normalize(abs(x_95));
  x_GLF_color = vec4<f32>(x_97.x, x_97.y, x_97.z, 1.0);
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
