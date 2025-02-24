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

alias Arr_1 = array<strided_arr_1, 1u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr_1,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(1) var<uniform> x_6 : buf1;

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_9 : buf0;

fn main_1() {
  var icoord : vec2<i32>;
  var x_40 : f32;
  var icoord_1 : vec2<i32>;
  let x_42 : f32 = gl_FragCoord.x;
  let x_44 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  let x_47 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  if (((x_42 * x_44) > x_47)) {
    let x_52 : vec4<f32> = gl_FragCoord;
    let x_55 : f32 = x_6.x_GLF_uniform_float_values[1].el;
    let x_58 : f32 = x_6.x_GLF_uniform_float_values[0].el;
    let x_60 : f32 = x_6.x_GLF_uniform_float_values[2].el;
    icoord = vec2<i32>(((vec2<f32>(x_52.x, x_52.y) * x_55) - vec2<f32>(x_58, x_60)));
    let x_65 : f32 = x_6.x_GLF_uniform_float_values[2].el;
    let x_67 : f32 = x_6.x_GLF_uniform_float_values[3].el;
    let x_69 : i32 = icoord.x;
    let x_71 : i32 = icoord.y;
    let x_74 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    if (((x_69 * x_71) != x_74)) {
      let x_80 : f32 = x_6.x_GLF_uniform_float_values[3].el;
      x_40 = x_80;
    } else {
      let x_82 : f32 = x_6.x_GLF_uniform_float_values[2].el;
      x_40 = x_82;
    }
    let x_83 : f32 = x_40;
    let x_85 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(x_65, x_67, x_83, f32(x_85));
  } else {
    let x_88 : vec4<f32> = gl_FragCoord;
    let x_91 : f32 = x_6.x_GLF_uniform_float_values[1].el;
    let x_94 : f32 = x_6.x_GLF_uniform_float_values[0].el;
    let x_96 : f32 = x_6.x_GLF_uniform_float_values[2].el;
    icoord_1 = vec2<i32>(((vec2<f32>(x_88.x, x_88.y) * x_91) - vec2<f32>(x_94, x_96)));
    let x_101 : f32 = x_6.x_GLF_uniform_float_values[3].el;
    let x_103 : f32 = x_6.x_GLF_uniform_float_values[3].el;
    let x_105 : i32 = icoord_1.x;
    let x_108 : f32 = x_6.x_GLF_uniform_float_values[3].el;
    x_GLF_color = vec4<f32>(x_101, x_103, f32(x_105), x_108);
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
