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
  var a : i32;
  var i : i32;
  var j : i32;
  let x_36 : i32 = x_7.x_GLF_uniform_int_values[2].el;
  a = x_36;
  let x_38 : i32 = x_7.x_GLF_uniform_int_values[2].el;
  i = x_38;
  loop {
    let x_43 : i32 = i;
    let x_45 : i32 = x_7.x_GLF_uniform_int_values[0].el;
    if ((x_43 < x_45)) {
    } else {
      break;
    }
    let x_49 : i32 = x_7.x_GLF_uniform_int_values[2].el;
    j = x_49;
    loop {
      let x_54 : i32 = j;
      let x_56 : i32 = x_7.x_GLF_uniform_int_values[0].el;
      if ((x_54 < x_56)) {
      } else {
        break;
      }
      loop {
        let x_64 : i32 = x_7.x_GLF_uniform_int_values[1].el;
        a = x_64;
        let x_66 : f32 = gl_FragCoord.y;
        let x_68 : f32 = x_11.x_GLF_uniform_float_values[0].el;
        if ((x_66 < x_68)) {
          discard;
        }

        continuing {
          let x_72 : i32 = a;
          let x_74 : i32 = x_7.x_GLF_uniform_int_values[1].el;
          break if !(x_72 < x_74);
        }
      }
      let x_77 : f32 = gl_FragCoord.y;
      let x_79 : f32 = x_11.x_GLF_uniform_float_values[0].el;
      if ((x_77 < x_79)) {
        break;
      }

      continuing {
        let x_83 : i32 = j;
        j = (x_83 + 1);
      }
    }

    continuing {
      let x_85 : i32 = i;
      i = (x_85 + 1);
    }
  }
  let x_87 : i32 = a;
  let x_89 : i32 = x_7.x_GLF_uniform_int_values[1].el;
  if ((x_87 == x_89)) {
    let x_94 : i32 = a;
    let x_97 : i32 = x_7.x_GLF_uniform_int_values[2].el;
    let x_100 : i32 = x_7.x_GLF_uniform_int_values[2].el;
    let x_102 : i32 = a;
    x_GLF_color = vec4<f32>(f32(x_94), f32(x_97), f32(x_100), f32(x_102));
  } else {
    let x_106 : i32 = x_7.x_GLF_uniform_int_values[2].el;
    let x_107 : f32 = f32(x_106);
    x_GLF_color = vec4<f32>(x_107, x_107, x_107, x_107);
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
