struct buf0 {
  resolution : vec2<f32>,
}

struct buf1 {
  injectionSwitch : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_13 : buf0;

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(1) var<uniform> x_19 : buf1;

var<private> x_GLF_color : vec4<f32>;

fn compute_value_f1_f1_(limit : ptr<function, f32>, thirty_two : ptr<function, f32>) -> f32 {
  var result : f32;
  var i : i32;
  result = -0.5;
  i = 1;
  loop {
    let x_144 : i32 = i;
    if ((x_144 < 800)) {
    } else {
      break;
    }
    let x_147 : i32 = i;
    if (((x_147 % 32) == 0)) {
      let x_153 : f32 = result;
      result = (x_153 + 0.400000006);
    } else {
      let x_155 : i32 = i;
      let x_157 : f32 = *(thirty_two);
      if (((f32(x_155) - (round(x_157) * floor((f32(x_155) / round(x_157))))) <= 0.01)) {
        let x_163 : f32 = result;
        result = (x_163 + 100.0);
      }
    }
    let x_165 : i32 = i;
    let x_167 : f32 = *(limit);
    if ((f32(x_165) >= x_167)) {
      let x_171 : f32 = result;
      return x_171;
    }

    continuing {
      let x_172 : i32 = i;
      i = (x_172 + 1);
    }
  }
  let x_174 : f32 = result;
  return x_174;
}

fn main_1() {
  var c : vec3<f32>;
  var thirty_two_1 : f32;
  var param : f32;
  var param_1 : f32;
  var param_2 : f32;
  var param_3 : f32;
  var x_61 : vec3<f32>;
  var i_1 : i32;
  var j : f32;
  c = vec3<f32>(7.0, 8.0, 9.0);
  let x_63 : f32 = x_13.resolution.x;
  thirty_two_1 = round((x_63 / 8.0));
  let x_67 : f32 = gl_FragCoord.x;
  param = x_67;
  let x_68 : f32 = thirty_two_1;
  param_1 = x_68;
  let x_69 : f32 = compute_value_f1_f1_(&(param), &(param_1));
  c.x = x_69;
  let x_72 : f32 = gl_FragCoord.y;
  param_2 = x_72;
  let x_73 : f32 = thirty_two_1;
  param_3 = x_73;
  let x_74 : f32 = compute_value_f1_f1_(&(param_2), &(param_3));
  c.y = x_74;
  let x_77 : f32 = c.x;
  if (true) {
    let x_81 : vec3<f32> = c;
    x_61 = x_81;
  } else {
    let x_82 : vec3<f32> = c;
    let x_84 : f32 = x_19.injectionSwitch.x;
    x_61 = (x_82 * x_84);
  }
  let x_87 : f32 = x_61.y;
  c.z = (x_77 + x_87);
  i_1 = 0;
  loop {
    let x_94 : i32 = i_1;
    if ((x_94 < 3)) {
    } else {
      break;
    }
    let x_97 : i32 = i_1;
    let x_99 : f32 = c[x_97];
    if ((x_99 >= 1.0)) {
      let x_103 : i32 = i_1;
      let x_104 : i32 = i_1;
      let x_106 : f32 = c[x_104];
      let x_107 : i32 = i_1;
      let x_109 : f32 = c[x_107];
      c[x_103] = (x_106 * x_109);
    }
    j = 0.0;
    loop {
      let x_117 : f32 = x_19.injectionSwitch.x;
      let x_119 : f32 = x_19.injectionSwitch.y;
      if ((x_117 > x_119)) {
      } else {
        break;
      }
      let x_122 : f32 = j;
      let x_124 : f32 = x_19.injectionSwitch.x;
      if ((x_122 >= x_124)) {
        break;
      }
      let x_128 : f32 = j;
      j = (x_128 + 1.0);
    }

    continuing {
      let x_130 : i32 = i_1;
      i_1 = (x_130 + 1);
    }
  }
  let x_132 : vec3<f32> = c;
  let x_134 : vec3<f32> = normalize(abs(x_132));
  x_GLF_color = vec4<f32>(x_134.x, x_134.y, x_134.z, 1.0);
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
