struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 2u>;

struct buf1 {
  x_GLF_uniform_float_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : i32,
}

alias Arr_1 = array<strided_arr_1, 3u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr_1,
}

struct buf2 {
  injectionSwitch : vec2<f32>,
}

var<private> x_GLF_global_loop_count : i32;

@group(0) @binding(1) var<uniform> x_7 : buf1;

@group(0) @binding(0) var<uniform> x_10 : buf0;

@group(0) @binding(2) var<uniform> x_12 : buf2;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var f : f32;
  var r : i32;
  x_GLF_global_loop_count = 0;
  let x_42 : f32 = x_7.x_GLF_uniform_float_values[0].el;
  f = x_42;
  let x_44 : i32 = x_10.x_GLF_uniform_int_values[1].el;
  r = x_44;
  loop {
    let x_49 : i32 = r;
    let x_51 : i32 = x_10.x_GLF_uniform_int_values[2].el;
    if ((x_49 < x_51)) {
    } else {
      break;
    }
    let x_54 : i32 = x_GLF_global_loop_count;
    x_GLF_global_loop_count = (x_54 + 1);
    let x_57 : vec2<f32> = x_12.injectionSwitch;
    let x_60 : f32 = f;
    f = (x_60 + dpdx(x_57).y);

    continuing {
      let x_62 : i32 = r;
      r = (x_62 + 1);
    }
  }
  loop {
    let x_68 : i32 = x_GLF_global_loop_count;
    if ((x_68 < 100)) {
    } else {
      break;
    }
    let x_71 : i32 = x_GLF_global_loop_count;
    x_GLF_global_loop_count = (x_71 + 1);
    let x_74 : f32 = x_7.x_GLF_uniform_float_values[0].el;
    let x_75 : f32 = f;
    f = (x_75 + x_74);
  }
  let x_77 : f32 = f;
  let x_79 : f32 = x_7.x_GLF_uniform_float_values[1].el;
  if ((x_77 == x_79)) {
    let x_85 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    let x_88 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_91 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_94 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_85), f32(x_88), f32(x_91), f32(x_94));
  } else {
    let x_98 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_99 : f32 = f32(x_98);
    x_GLF_color = vec4<f32>(x_99, x_99, x_99, x_99);
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
