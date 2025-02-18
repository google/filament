struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 2u>;

struct buf1 {
  x_GLF_uniform_float_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : i32,
}

alias Arr_1 = array<strided_arr_1, 4u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr_1,
}

@group(0) @binding(1) var<uniform> x_7 : buf1;

@group(0) @binding(0) var<uniform> x_10 : buf0;

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var m23 : mat2x3<f32>;
  var i : i32;
  let x_46 : f32 = x_7.x_GLF_uniform_float_values[1].el;
  m23 = mat2x3<f32>(vec3<f32>(x_46, 0.0, 0.0), vec3<f32>(0.0, x_46, 0.0));
  i = 1;
  loop {
    var x_80 : bool;
    var x_81_phi : bool;
    let x_54 : i32 = i;
    let x_56 : i32 = x_10.x_GLF_uniform_int_values[3].el;
    if ((x_54 < x_56)) {
    } else {
      break;
    }
    let x_60 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    let x_62 : i32 = x_10.x_GLF_uniform_int_values[2].el;
    let x_64 : f32 = x_7.x_GLF_uniform_float_values[0].el;
    let x_66 : f32 = m23[x_60][x_62];
    m23[x_60][x_62] = (x_66 + x_64);
    let x_70 : f32 = gl_FragCoord.y;
    let x_72 : f32 = x_7.x_GLF_uniform_float_values[0].el;
    if ((x_70 < x_72)) {
    }
    x_81_phi = true;
    if (true) {
      let x_79 : f32 = gl_FragCoord.x;
      x_80 = (x_79 < 0.0);
      x_81_phi = x_80;
    }
    let x_81 : bool = x_81_phi;
    if (!(x_81)) {
      break;
    }

    continuing {
      let x_85 : i32 = i;
      i = (x_85 + 1);
    }
  }
  let x_87 : mat2x3<f32> = m23;
  let x_89 : i32 = x_10.x_GLF_uniform_int_values[1].el;
  let x_92 : i32 = x_10.x_GLF_uniform_int_values[1].el;
  let x_95 : i32 = x_10.x_GLF_uniform_int_values[1].el;
  let x_98 : i32 = x_10.x_GLF_uniform_int_values[1].el;
  let x_101 : i32 = x_10.x_GLF_uniform_int_values[1].el;
  let x_104 : i32 = x_10.x_GLF_uniform_int_values[0].el;
  let x_108 : mat2x3<f32> = mat2x3<f32>(vec3<f32>(f32(x_89), f32(x_92), f32(x_95)), vec3<f32>(f32(x_98), f32(x_101), f32(x_104)));
  if ((all((x_87[0u] == x_108[0u])) & all((x_87[1u] == x_108[1u])))) {
    let x_122 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    let x_125 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_128 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_131 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_122), f32(x_125), f32(x_128), f32(x_131));
  } else {
    let x_135 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_136 : f32 = f32(x_135);
    x_GLF_color = vec4<f32>(x_136, x_136, x_136, x_136);
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
