struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 3u>;

struct buf1 {
  x_GLF_uniform_float_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : i32,
}

alias Arr_1 = array<strided_arr_1, 4u>;

struct buf2 {
  x_GLF_uniform_int_values : Arr_1,
}

struct buf3 {
  three : i32,
}

struct strided_arr_2 {
  @size(16)
  el : u32,
}

alias Arr_2 = array<strided_arr_2, 1u>;

struct buf0 {
  x_GLF_uniform_uint_values : Arr_2,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(1) var<uniform> x_8 : buf1;

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(2) var<uniform> x_12 : buf2;

@group(0) @binding(3) var<uniform> x_14 : buf3;

@group(0) @binding(0) var<uniform> x_16 : buf0;

fn func0_() {
  var tmp : vec4<f32>;
  let x_112 : f32 = gl_FragCoord.x;
  let x_114 : f32 = x_8.x_GLF_uniform_float_values[1].el;
  if ((x_112 > x_114)) {
    let x_118 : vec4<f32> = x_GLF_color;
    tmp = x_118;
  }
  let x_119 : vec4<f32> = tmp;
  x_GLF_color = x_119;
  return;
}

fn func1_() -> i32 {
  var a : i32;
  let x_122 : i32 = x_12.x_GLF_uniform_int_values[1].el;
  a = x_122;
  loop {
    let x_127 : i32 = a;
    let x_129 : i32 = x_12.x_GLF_uniform_int_values[3].el;
    if ((x_127 < x_129)) {
    } else {
      break;
    }
    let x_133 : i32 = x_14.three;
    let x_135 : i32 = x_12.x_GLF_uniform_int_values[1].el;
    if ((x_133 > x_135)) {
      func0_();
      let x_142 : i32 = x_12.x_GLF_uniform_int_values[3].el;
      a = x_142;
    } else {
      func0_();
    }
  }
  let x_144 : i32 = a;
  return x_144;
}

fn main_1() {
  var a_1 : i32;
  var i : i32;
  var j : i32;
  let x_56 : f32 = gl_FragCoord.x;
  let x_58 : f32 = x_8.x_GLF_uniform_float_values[1].el;
  if ((x_56 > x_58)) {
    let x_64 : f32 = x_8.x_GLF_uniform_float_values[0].el;
    let x_66 : f32 = x_8.x_GLF_uniform_float_values[1].el;
    let x_68 : f32 = x_8.x_GLF_uniform_float_values[0].el;
    let x_70 : f32 = x_8.x_GLF_uniform_float_values[2].el;
    x_GLF_color = vec4<f32>(x_64, x_66, x_68, x_70);
  } else {
    let x_73 : u32 = x_16.x_GLF_uniform_uint_values[0].el;
    x_GLF_color = unpack4x8snorm(x_73);
  }
  let x_76 : i32 = x_12.x_GLF_uniform_int_values[2].el;
  a_1 = x_76;
  i = 0;
  loop {
    let x_81 : i32 = i;
    if ((x_81 < 5)) {
    } else {
      break;
    }
    j = 0;
    loop {
      let x_88 : i32 = j;
      if ((x_88 < 2)) {
      } else {
        break;
      }
      let x_91 : i32 = func1_();
      let x_92 : i32 = a_1;
      a_1 = (x_92 + x_91);

      continuing {
        let x_94 : i32 = j;
        j = (x_94 + 1);
      }
    }

    continuing {
      let x_96 : i32 = i;
      i = (x_96 + 1);
    }
  }
  let x_98 : i32 = a_1;
  let x_100 : i32 = x_12.x_GLF_uniform_int_values[0].el;
  if ((x_98 == x_100)) {
    let x_105 : f32 = x_8.x_GLF_uniform_float_values[0].el;
    let x_107 : f32 = x_GLF_color.z;
    x_GLF_color.z = (x_107 - x_105);
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
