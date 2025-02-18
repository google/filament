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

alias Arr_1 = array<strided_arr_1, 3u>;

struct buf1 {
  x_GLF_uniform_int_values : Arr_1,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

@group(0) @binding(1) var<uniform> x_9 : buf1;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var sums : array<f32, 3u>;
  var i : i32;
  var indexable : mat2x4<f32>;
  let x_40 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  let x_42 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  let x_44 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  sums = array<f32, 3u>(x_40, x_42, x_44);
  i = 0;
  loop {
    let x_50 : i32 = i;
    let x_52 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    let x_54 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    if ((x_50 < clamp(x_52, x_54, 2))) {
    } else {
      break;
    }
    let x_59 : i32 = x_9.x_GLF_uniform_int_values[2].el;
    let x_61 : f32 = x_6.x_GLF_uniform_float_values[0].el;
    let x_65 : i32 = i;
    let x_67 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    indexable = mat2x4<f32>(vec4<f32>(x_61, 0.0, 0.0, 0.0), vec4<f32>(0.0, x_61, 0.0, 0.0));
    let x_69 : f32 = indexable[x_65][x_67];
    let x_71 : f32 = sums[x_59];
    sums[x_59] = (x_71 + x_69);

    continuing {
      let x_74 : i32 = i;
      i = (x_74 + 1);
    }
  }
  let x_77 : i32 = x_9.x_GLF_uniform_int_values[2].el;
  let x_79 : f32 = sums[x_77];
  let x_81 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  if ((x_79 == x_81)) {
    let x_87 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    let x_90 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    let x_93 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    let x_96 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_87), f32(x_90), f32(x_93), f32(x_96));
  } else {
    let x_100 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    let x_101 : f32 = f32(x_100);
    x_GLF_color = vec4<f32>(x_101, x_101, x_101, x_101);
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
