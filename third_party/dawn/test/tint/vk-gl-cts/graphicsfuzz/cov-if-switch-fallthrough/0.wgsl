struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 2u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : i32,
}

alias Arr_1 = array<strided_arr_1, 2u>;

struct buf1 {
  x_GLF_uniform_int_values : Arr_1,
}

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(1) var<uniform> x_8 : buf1;

fn main_1() {
  let x_31 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  x_GLF_color = vec4<f32>(x_31, x_31, x_31, x_31);
  let x_34 : f32 = gl_FragCoord.y;
  let x_36 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  if ((x_34 >= x_36)) {
    let x_41 : i32 = x_8.x_GLF_uniform_int_values[1].el;
    switch(x_41) {
      case 0, 16: {
        let x_45 : i32 = x_8.x_GLF_uniform_int_values[0].el;
        let x_46 : f32 = f32(x_45);
        let x_47 : f32 = f32(x_41);
        x_GLF_color = vec4<f32>(x_46, x_47, x_47, x_46);
      }
      default: {
      }
    }
  }
  let x_50 : i32 = x_8.x_GLF_uniform_int_values[1].el;
  let x_52 : i32 = x_8.x_GLF_uniform_int_values[0].el;
  if ((x_50 == x_52)) {
    x_GLF_color = vec4<f32>(x_36, x_36, x_36, x_36);
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
