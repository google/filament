struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 5u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : f32,
}

alias Arr_1 = array<strided_arr_1, 3u>;

struct buf1 {
  x_GLF_uniform_float_values : Arr_1,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

@group(0) @binding(1) var<uniform> x_9 : buf1;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var I : vec4<f32>;
  var N : vec4<f32>;
  var R : vec4<f32>;
  var r : vec4<f32>;
  let x_40 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_43 : i32 = x_6.x_GLF_uniform_int_values[3].el;
  let x_46 : i32 = x_6.x_GLF_uniform_int_values[4].el;
  I = bitcast<vec4<f32>>(vec4<u32>(bitcast<u32>(x_40), bitcast<u32>(x_43), bitcast<u32>(x_46), 92985u));
  let x_51 : f32 = x_9.x_GLF_uniform_float_values[1].el;
  N = vec4<f32>(x_51, x_51, x_51, x_51);
  let x_53 : vec4<f32> = I;
  R = reflect(x_53, vec4<f32>(0.5, 0.5, 0.5, 0.5));
  let x_55 : vec4<f32> = I;
  let x_57 : f32 = x_9.x_GLF_uniform_float_values[2].el;
  let x_58 : vec4<f32> = N;
  let x_59 : vec4<f32> = I;
  let x_62 : vec4<f32> = N;
  r = (x_55 - (x_62 * (x_57 * dot(x_58, x_59))));
  let x_65 : vec4<f32> = R;
  let x_66 : vec4<f32> = r;
  let x_69 : f32 = x_9.x_GLF_uniform_float_values[0].el;
  if ((distance(x_65, x_66) < x_69)) {
    let x_75 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_78 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_81 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_84 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_75), f32(x_78), f32(x_81), f32(x_84));
  } else {
    let x_88 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_89 : f32 = f32(x_88);
    x_GLF_color = vec4<f32>(x_89, x_89, x_89, x_89);
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
