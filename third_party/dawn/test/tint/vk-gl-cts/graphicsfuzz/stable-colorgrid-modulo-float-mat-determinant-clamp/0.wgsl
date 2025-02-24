struct buf0 {
  resolution : vec2<f32>,
}

@group(0) @binding(0) var<uniform> x_13 : buf0;

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn compute_value_f1_f1_(limit : ptr<function, f32>, thirty_two : ptr<function, f32>) -> f32 {
  var result : f32;
  var i : i32;
  result = -0.5;
  i = 1;
  loop {
    let x_111 : i32 = i;
    if ((x_111 < 800)) {
    } else {
      break;
    }
    let x_114 : i32 = i;
    if (((x_114 % 32) == 0)) {
      let x_120 : f32 = result;
      result = (x_120 + 0.400000006);
    } else {
      let x_122 : i32 = i;
      let x_124 : f32 = *(thirty_two);
      if (((f32(x_122) - (round(x_124) * floor((f32(x_122) / round(x_124))))) <= 0.01)) {
        let x_130 : f32 = result;
        result = (x_130 + 100.0);
      }
    }
    let x_132 : i32 = i;
    let x_134 : f32 = *(limit);
    if ((f32(x_132) >= x_134)) {
      let x_138 : f32 = result;
      return x_138;
    }

    continuing {
      let x_139 : i32 = i;
      i = (x_139 + 1);
    }
  }
  let x_141 : f32 = result;
  return x_141;
}

fn main_1() {
  var c : vec3<f32>;
  var thirty_two_1 : f32;
  var param : f32;
  var param_1 : f32;
  var param_2 : f32;
  var param_3 : f32;
  var i_1 : i32;
  c = vec3<f32>(7.0, 8.0, 9.0);
  let x_56 : f32 = x_13.resolution.x;
  thirty_two_1 = round((x_56 / 8.0));
  let x_60 : f32 = gl_FragCoord.x;
  param = x_60;
  let x_61 : f32 = thirty_two_1;
  param_1 = x_61;
  let x_62 : f32 = compute_value_f1_f1_(&(param), &(param_1));
  c.x = x_62;
  let x_65 : f32 = gl_FragCoord.y;
  param_2 = x_65;
  let x_66 : f32 = thirty_two_1;
  param_3 = x_66;
  let x_67 : f32 = compute_value_f1_f1_(&(param_2), &(param_3));
  c.y = x_67;
  let x_70 : f32 = c.x;
  let x_72 : f32 = c.y;
  c.z = (x_70 + x_72);
  i_1 = 0;
  loop {
    let x_79 : i32 = i_1;
    if ((x_79 < 3)) {
    } else {
      break;
    }
    let x_82 : i32 = i_1;
    let x_84 : f32 = c[x_82];
    if ((x_84 >= 1.0)) {
      let x_88 : i32 = i_1;
      let x_89 : i32 = i_1;
      let x_91 : f32 = c[x_89];
      let x_92 : i32 = i_1;
      let x_94 : f32 = c[x_92];
      c[x_88] = (x_91 * x_94);
    }

    continuing {
      let x_97 : i32 = i_1;
      i_1 = (x_97 + 1);
    }
  }
  let x_99 : vec3<f32> = c;
  let x_101 : vec3<f32> = normalize(abs(x_99));
  x_GLF_color = vec4<f32>(x_101.x, x_101.y, x_101.z, 1.0);
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
