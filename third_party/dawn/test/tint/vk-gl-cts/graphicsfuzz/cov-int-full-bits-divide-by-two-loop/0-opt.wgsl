struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 2u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : i32;
  var i : i32;
  let x_32 : f32 = gl_FragCoord.x;
  let x_35 : i32 = x_7.x_GLF_uniform_int_values[1].el;
  a = select(-1, 0, (i32(x_32) < x_35));
  i = 0;
  loop {
    let x_42 : i32 = i;
    if ((x_42 < 5)) {
    } else {
      break;
    }
    let x_45 : i32 = a;
    a = (x_45 / 2);

    continuing {
      let x_47 : i32 = i;
      i = (x_47 + 1);
    }
  }
  let x_49 : i32 = a;
  if ((x_49 == 0)) {
    let x_55 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    let x_58 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    let x_61 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    let x_64 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_55), f32(x_58), f32(x_61), f32(x_64));
  } else {
    let x_68 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    let x_69 : f32 = f32(x_68);
    x_GLF_color = vec4<f32>(x_69, x_69, x_69, x_69);
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
