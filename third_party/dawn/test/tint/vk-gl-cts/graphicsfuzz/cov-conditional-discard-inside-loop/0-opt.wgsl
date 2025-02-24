struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 3u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

struct buf2 {
  injectionSwitch : vec2<f32>,
}

struct strided_arr_1 {
  @size(16)
  el : f32,
}

alias Arr_1 = array<strided_arr_1, 1u>;

struct buf1 {
  x_GLF_uniform_float_values : Arr_1,
}

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_6 : buf0;

@group(0) @binding(2) var<uniform> x_9 : buf2;

@group(0) @binding(1) var<uniform> x_11 : buf1;

fn main_1() {
  var a : i32;
  var i : i32;
  a = 1;
  let x_38 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  let x_41 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_44 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_47 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  x_GLF_color = vec4<f32>(f32(x_38), f32(x_41), f32(x_44), f32(x_47));
  let x_51 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  i = x_51;
  loop {
    let x_56 : i32 = i;
    let x_58 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    if ((x_56 < x_58)) {
    } else {
      break;
    }
    let x_61 : i32 = a;
    a = (x_61 + 1);
    if ((x_61 > 3)) {
      break;
    }
    let x_67 : f32 = x_9.injectionSwitch.x;
    let x_69 : f32 = x_11.x_GLF_uniform_float_values[0].el;
    if ((x_67 > x_69)) {
      discard;
    }

    continuing {
      let x_73 : i32 = i;
      i = (x_73 + 1);
    }
  }
  return;
}

struct main_out {
  @location(0)
  x_GLF_color_1 : vec4<f32>,
}

@fragment
fn main() -> main_out {
  main_1();
  return main_out(x_GLF_color);
}
