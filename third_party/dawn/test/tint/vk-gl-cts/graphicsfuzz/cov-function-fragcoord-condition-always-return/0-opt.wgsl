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

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_11 : buf0;

fn func_f1_(x : ptr<function, f32>) -> f32 {
  loop {
    if (true) {
    } else {
      break;
    }
    loop {
      let x_77 : f32 = gl_FragCoord.y;
      let x_79 : f32 = x_8.x_GLF_uniform_float_values[2].el;
      if ((x_77 < x_79)) {
        loop {

          continuing {
            let x_88 : f32 = gl_FragCoord.x;
            let x_90 : f32 = x_8.x_GLF_uniform_float_values[2].el;
            break if !(x_88 < x_90);
          }
        }
      }
      let x_92 : f32 = *(x);
      let x_94 : f32 = x_8.x_GLF_uniform_float_values[3].el;
      if ((x_92 < x_94)) {
        let x_99 : f32 = x_8.x_GLF_uniform_float_values[1].el;
        return x_99;
      }

      continuing {
        let x_101 : f32 = gl_FragCoord.y;
        let x_103 : f32 = x_8.x_GLF_uniform_float_values[2].el;
        break if !(x_101 < x_103);
      }
    }
  }
  let x_106 : f32 = x_8.x_GLF_uniform_float_values[0].el;
  return x_106;
}

fn main_1() {
  var param : f32;
  let x_41 : f32 = gl_FragCoord.x;
  param = x_41;
  let x_42 : f32 = func_f1_(&(param));
  let x_44 : f32 = x_8.x_GLF_uniform_float_values[1].el;
  if ((x_42 == x_44)) {
    let x_50 : i32 = x_11.x_GLF_uniform_int_values[0].el;
    let x_53 : i32 = x_11.x_GLF_uniform_int_values[1].el;
    let x_56 : i32 = x_11.x_GLF_uniform_int_values[1].el;
    let x_59 : i32 = x_11.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_50), f32(x_53), f32(x_56), f32(x_59));
  } else {
    let x_63 : i32 = x_11.x_GLF_uniform_int_values[1].el;
    let x_64 : f32 = f32(x_63);
    x_GLF_color = vec4<f32>(x_64, x_64, x_64, x_64);
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
