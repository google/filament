struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 4u>;

struct buf1 {
  x_GLF_uniform_float_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : i32,
}

alias Arr_1 = array<strided_arr_1, 2u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr_1,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(1) var<uniform> x_8 : buf1;

@group(0) @binding(0) var<uniform> x_12 : buf0;

var<private> x_GLF_v1 : vec4<f32>;

fn main_1() {
  var uv : vec2<f32>;
  var v1 : vec4<f32>;
  var a : f32;
  var i : i32;
  let x_49 : vec4<f32> = gl_FragCoord;
  uv = vec2<f32>(x_49.x, x_49.y);
  let x_52 : f32 = x_8.x_GLF_uniform_float_values[0].el;
  v1 = vec4<f32>(x_52, x_52, x_52, x_52);
  let x_55 : f32 = uv.y;
  let x_57 : f32 = x_8.x_GLF_uniform_float_values[0].el;
  if ((x_55 >= x_57)) {
    let x_62 : f32 = x_8.x_GLF_uniform_float_values[2].el;
    v1.x = x_62;
    let x_65 : f32 = x_8.x_GLF_uniform_float_values[0].el;
    v1.y = x_65;
    let x_68 : f32 = x_8.x_GLF_uniform_float_values[0].el;
    v1.z = x_68;
    let x_71 : f32 = x_8.x_GLF_uniform_float_values[2].el;
    v1.w = x_71;
  }
  let x_74 : f32 = x_8.x_GLF_uniform_float_values[2].el;
  a = x_74;
  let x_15 : i32 = x_12.x_GLF_uniform_int_values[1].el;
  i = x_15;
  loop {
    let x_16 : i32 = i;
    let x_17 : i32 = x_12.x_GLF_uniform_int_values[0].el;
    if ((x_16 < x_17)) {
    } else {
      break;
    }
    let x_84 : f32 = x_8.x_GLF_uniform_float_values[2].el;
    let x_86 : f32 = x_8.x_GLF_uniform_float_values[0].el;
    if ((x_84 < x_86)) {
      discard;
    }
    let x_91 : f32 = v1.x;
    let x_93 : f32 = v1.y;
    let x_96 : f32 = v1.z;
    let x_99 : f32 = v1.w;
    let x_102 : f32 = x_8.x_GLF_uniform_float_values[3].el;
    a = pow((((x_91 + x_93) + x_96) + x_99), x_102);

    continuing {
      let x_18 : i32 = i;
      i = (x_18 + 1);
    }
  }
  let x_104 : f32 = a;
  let x_106 : f32 = x_8.x_GLF_uniform_float_values[1].el;
  if ((x_104 == x_106)) {
    let x_111 : vec4<f32> = v1;
    x_GLF_v1 = x_111;
  } else {
    let x_20 : i32 = x_12.x_GLF_uniform_int_values[1].el;
    let x_113 : f32 = f32(x_20);
    x_GLF_v1 = vec4<f32>(x_113, x_113, x_113, x_113);
  }
  return;
}

struct main_out {
  @location(0)
  x_GLF_v1_1 : vec4<f32>,
}

@fragment
fn main(@builtin(position) gl_FragCoord_param : vec4<f32>) -> main_out {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  return main_out(x_GLF_v1);
}
