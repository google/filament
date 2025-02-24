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

alias Arr_1 = array<strided_arr_1, 2u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr_1,
}

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(1) var<uniform> x_5 : buf1;

@group(0) @binding(0) var<uniform> x_8 : buf0;

fn main_1() {
  var i : i32;
  let x_29 : i32 = x_5.x_GLF_uniform_int_values[0].el;
  let x_30 : f32 = f32(x_29);
  x_GLF_color = vec4<f32>(x_30, x_30, x_30, x_30);
  let x_33 : i32 = x_5.x_GLF_uniform_int_values[0].el;
  i = x_33;
  loop {
    let x_38 : i32 = i;
    let x_40 : i32 = x_5.x_GLF_uniform_int_values[1].el;
    if ((x_38 < x_40)) {
    } else {
      break;
    }
    let x_44 : f32 = x_8.x_GLF_uniform_float_values[1].el;
    let x_45 : i32 = i;
    if (!((x_44 <= f32(x_45)))) {
      let x_52 : f32 = x_8.x_GLF_uniform_float_values[0].el;
      let x_53 : i32 = i;
      let x_55 : i32 = i;
      let x_58 : f32 = x_8.x_GLF_uniform_float_values[0].el;
      let x_60 : vec4<f32> = x_GLF_color;
      x_GLF_color = (x_60 + vec4<f32>(x_52, f32(x_53), f32(x_55), x_58));
    }

    continuing {
      let x_62 : i32 = i;
      i = (x_62 + 1);
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
