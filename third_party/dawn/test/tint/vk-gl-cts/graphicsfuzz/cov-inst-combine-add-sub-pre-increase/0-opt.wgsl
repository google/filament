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

@group(0) @binding(1) var<uniform> x_8 : buf1;

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_12 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn f1_() -> i32 {
  var i : i32;
  var A : array<i32, 10u>;
  var a : i32;
  let x_56 : i32 = x_8.x_GLF_uniform_int_values[2].el;
  i = x_56;
  loop {
    let x_61 : i32 = i;
    let x_63 : i32 = x_8.x_GLF_uniform_int_values[0].el;
    if ((x_61 < x_63)) {
    } else {
      break;
    }
    let x_66 : i32 = i;
    let x_68 : i32 = x_8.x_GLF_uniform_int_values[2].el;
    A[x_66] = x_68;

    continuing {
      let x_70 : i32 = i;
      i = (x_70 + 1);
    }
  }
  a = -1;
  let x_73 : f32 = gl_FragCoord.y;
  let x_75 : f32 = x_12.x_GLF_uniform_float_values[0].el;
  if ((x_73 >= x_75)) {
    let x_79 : i32 = a;
    let x_80 : i32 = (x_79 + 1);
    a = x_80;
    let x_82 : i32 = x_8.x_GLF_uniform_int_values[1].el;
    A[x_80] = x_82;
  }
  let x_85 : i32 = x_8.x_GLF_uniform_int_values[2].el;
  let x_87 : i32 = A[x_85];
  let x_89 : i32 = x_8.x_GLF_uniform_int_values[1].el;
  if ((x_87 == x_89)) {
    let x_94 : i32 = a;
    let x_95 : i32 = (x_94 + 1);
    a = x_95;
    let x_97 : i32 = A[x_95];
    return x_97;
  } else {
    let x_99 : i32 = x_8.x_GLF_uniform_int_values[1].el;
    return x_99;
  }
}

fn main_1() {
  var i_1 : i32;
  let x_42 : i32 = f1_();
  i_1 = x_42;
  let x_44 : i32 = x_8.x_GLF_uniform_int_values[1].el;
  let x_46 : i32 = i_1;
  let x_48 : i32 = i_1;
  let x_51 : i32 = x_8.x_GLF_uniform_int_values[1].el;
  x_GLF_color = vec4<f32>(f32(x_44), f32(x_46), f32(x_48), f32(x_51));
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
