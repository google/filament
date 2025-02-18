struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 2u>;

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

struct buf2 {
  injectionSwitch : vec2<f32>,
}

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(1) var<uniform> x_7 : buf1;

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_9 : buf0;

@group(0) @binding(2) var<uniform> x_11 : buf2;

fn main_1() {
  var i : i32;
  x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  let x_42 : i32 = x_7.x_GLF_uniform_int_values[1].el;
  i = x_42;
  loop {
    let x_47 : i32 = i;
    let x_49 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    if ((x_47 < x_49)) {
    } else {
      break;
    }
    let x_52 : i32 = i;
    let x_54 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    if ((x_52 != x_54)) {
      return;
    }

    continuing {
      let x_58 : i32 = i;
      i = (x_58 + 1);
    }
  }
  let x_61 : f32 = gl_FragCoord.y;
  let x_63 : f32 = x_9.x_GLF_uniform_float_values[0].el;
  if ((x_61 < x_63)) {
    return;
  }
  let x_68 : f32 = x_11.injectionSwitch.y;
  x_GLF_color = vec4<f32>(vec3<f32>(1.0, 1.0, 1.0).x, vec3<f32>(1.0, 1.0, 1.0).y, vec3<f32>(1.0, 1.0, 1.0).z, x_68);
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
