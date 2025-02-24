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
  var v1 : vec4<f32>;
  var i : i32;
  var a : i32;
  var indexable : array<vec4<f32>, 2u>;
  var indexable_1 : array<vec4<f32>, 2u>;
  let x_45 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  v1 = vec4<f32>(x_45, x_45, x_45, x_45);
  let x_48 : i32 = x_9.x_GLF_uniform_int_values[1].el;
  i = x_48;
  loop {
    let x_53 : i32 = i;
    let x_55 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    if ((x_53 < x_55)) {
    } else {
      break;
    }
    let x_58 : i32 = i;
    let x_60 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    let x_62 : i32 = x_9.x_GLF_uniform_int_values[2].el;
    indexable = array<vec4<f32>, 2u>(vec4<f32>(1.0, 1.0, 1.0, 1.0), vec4<f32>(0.0, 0.0, 0.0, 0.0));
    let x_65 : f32 = indexable[clamp(x_58, x_60, x_62)].x;
    a = i32(x_65);
    let x_68 : f32 = x_6.x_GLF_uniform_float_values[0].el;
    let x_70 : f32 = x_6.x_GLF_uniform_float_values[1].el;
    let x_72 : f32 = x_6.x_GLF_uniform_float_values[1].el;
    let x_74 : f32 = x_6.x_GLF_uniform_float_values[0].el;
    let x_77 : f32 = x_6.x_GLF_uniform_float_values[1].el;
    let x_79 : f32 = x_6.x_GLF_uniform_float_values[0].el;
    let x_81 : f32 = x_6.x_GLF_uniform_float_values[0].el;
    let x_83 : f32 = x_6.x_GLF_uniform_float_values[1].el;
    let x_86 : i32 = a;
    indexable_1 = array<vec4<f32>, 2u>(vec4<f32>(x_68, x_70, x_72, x_74), vec4<f32>(x_77, x_79, x_81, x_83));
    let x_88 : vec4<f32> = indexable_1[x_86];
    v1 = x_88;

    continuing {
      let x_89 : i32 = i;
      i = (x_89 + 1);
    }
  }
  let x_91 : vec4<f32> = v1;
  x_GLF_color = x_91;
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
