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

alias Arr_1 = array<strided_arr_1, 2u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr_1,
}

@group(0) @binding(1) var<uniform> x_7 : buf1;

@group(0) @binding(0) var<uniform> x_9 : buf0;

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_v1 : vec4<f32>;

fn main_1() {
  var i : i32;
  var j : i32;
  let x_36 : i32 = x_7.x_GLF_uniform_int_values[1].el;
  i = x_36;
  loop {
    let x_41 : i32 = i;
    let x_43 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    if ((x_41 < x_43)) {
    } else {
      break;
    }
    let x_47 : f32 = x_9.x_GLF_uniform_float_values[0].el;
    let x_49 : f32 = x_9.x_GLF_uniform_float_values[1].el;
    if ((x_47 > x_49)) {
      discard;
    }
    let x_54 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    j = x_54;
    loop {
      let x_59 : i32 = j;
      let x_61 : i32 = x_7.x_GLF_uniform_int_values[0].el;
      if ((x_59 < x_61)) {
      } else {
        break;
      }
      let x_65 : f32 = gl_FragCoord.x;
      let x_67 : f32 = x_9.x_GLF_uniform_float_values[0].el;
      if ((x_65 < x_67)) {
        discard;
      }
      let x_72 : i32 = x_7.x_GLF_uniform_int_values[2].el;
      let x_75 : i32 = x_7.x_GLF_uniform_int_values[1].el;
      let x_78 : i32 = x_7.x_GLF_uniform_int_values[1].el;
      let x_81 : i32 = x_7.x_GLF_uniform_int_values[2].el;
      x_GLF_v1 = vec4<f32>(f32(x_72), f32(x_75), f32(x_78), f32(x_81));

      continuing {
        let x_84 : i32 = j;
        j = (x_84 + 1);
      }
    }

    continuing {
      let x_86 : i32 = i;
      i = (x_86 + 1);
    }
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
