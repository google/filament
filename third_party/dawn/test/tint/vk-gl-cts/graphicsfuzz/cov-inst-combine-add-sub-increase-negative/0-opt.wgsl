struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 3u>;

struct buf1 {
  x_GLF_uniform_int_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : f32,
}

alias Arr_1 = array<strided_arr_1, 1u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr_1,
}

@group(0) @binding(1) var<uniform> x_7 : buf1;

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_11 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var i : i32;
  var arr : array<i32, 2u>;
  var a : i32;
  let x_40 : i32 = x_7.x_GLF_uniform_int_values[0].el;
  i = x_40;
  loop {
    let x_45 : i32 = i;
    let x_47 : i32 = x_7.x_GLF_uniform_int_values[2].el;
    if ((x_45 < x_47)) {
    } else {
      break;
    }
    let x_50 : i32 = i;
    let x_52 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    arr[x_50] = x_52;

    continuing {
      let x_54 : i32 = i;
      i = (x_54 + 1);
    }
  }
  a = -1;
  let x_57 : f32 = gl_FragCoord.y;
  let x_59 : f32 = x_11.x_GLF_uniform_float_values[0].el;
  if (!((x_57 < x_59))) {
    let x_64 : i32 = a;
    let x_65 : i32 = (x_64 + 1);
    a = x_65;
    let x_67 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    arr[x_65] = x_67;
  }
  let x_69 : i32 = a;
  let x_70 : i32 = (x_69 + 1);
  a = x_70;
  let x_72 : i32 = x_7.x_GLF_uniform_int_values[2].el;
  arr[x_70] = x_72;
  let x_75 : i32 = x_7.x_GLF_uniform_int_values[0].el;
  let x_77 : i32 = arr[x_75];
  let x_79 : i32 = x_7.x_GLF_uniform_int_values[1].el;
  if ((x_77 == x_79)) {
    let x_84 : i32 = a;
    let x_87 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    let x_90 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    let x_92 : i32 = a;
    x_GLF_color = vec4<f32>(f32(x_84), f32(x_87), f32(x_90), f32(x_92));
  } else {
    let x_96 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    let x_97 : f32 = f32(x_96);
    x_GLF_color = vec4<f32>(x_97, x_97, x_97, x_97);
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
