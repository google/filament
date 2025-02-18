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
    let x_136 : i32 = i;
    if ((x_136 < 800)) {
    } else {
      break;
    }
    let x_139 : i32 = i;
    if (((x_139 % 32) == 0)) {
      let x_145 : f32 = result;
      result = (x_145 + 0.400000006);
    } else {
      let x_147 : i32 = i;
      let x_149 : f32 = *(thirty_two);
      if (((f32(x_147) - (round(x_149) * floor((f32(x_147) / round(x_149))))) <= 0.01)) {
        let x_155 : f32 = result;
        result = (x_155 + 100.0);
      }
    }
    let x_157 : i32 = i;
    let x_159 : f32 = *(limit);
    if ((f32(x_157) >= x_159)) {
      let x_163 : f32 = result;
      return x_163;
    }

    continuing {
      let x_164 : i32 = i;
      i = (x_164 + 1);
    }
  }
  let x_166 : f32 = result;
  return x_166;
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
  let x_76 : vec3<f32> = c;
  let x_79 : vec3<f32> = c;
  let x_87 : mat4x2<f32> = mat4x2<f32>(vec2<f32>(x_79.x, x_79.y), vec2<f32>(x_79.z, 1.0), vec2<f32>(1.0, 0.0), vec2<f32>(1.0, 0.0));
  c.z = (((x_76 * mat3x3<f32>(vec3<f32>(1.0, 0.0, 0.0), vec3<f32>(0.0, 1.0, 0.0), vec3<f32>(0.0, 0.0, 1.0)))).x + vec3<f32>(x_87[0u].x, x_87[0u].y, x_87[1u].x).y);
  i_1 = 0;
  loop {
    let x_99 : i32 = i_1;
    if ((x_99 < 3)) {
    } else {
      break;
    }
    let x_102 : i32 = i_1;
    let x_104 : f32 = c[x_102];
    if ((x_104 >= 1.0)) {
      let x_108 : i32 = i_1;
      let x_109 : i32 = i_1;
      let x_111 : f32 = c[x_109];
      let x_112 : i32 = i_1;
      let x_114 : f32 = c[x_112];
      c[x_108] = (x_111 * x_114);
      let x_118 : f32 = gl_FragCoord.y;
      if ((x_118 < 0.0)) {
        break;
      }
    }

    continuing {
      let x_122 : i32 = i_1;
      i_1 = (x_122 + 1);
    }
  }
  let x_124 : vec3<f32> = c;
  let x_126 : vec3<f32> = normalize(abs(x_124));
  x_GLF_color = vec4<f32>(x_126.x, x_126.y, x_126.z, 1.0);
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
