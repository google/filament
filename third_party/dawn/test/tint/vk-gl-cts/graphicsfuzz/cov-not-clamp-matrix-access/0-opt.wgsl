struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 5u>;

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

@group(0) @binding(1) var<uniform> x_10 : buf1;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var M1 : mat2x2<f32>;
  var a : f32;
  var c : i32;
  let x_41 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  let x_43 : f32 = x_6.x_GLF_uniform_float_values[2].el;
  let x_45 : f32 = x_6.x_GLF_uniform_float_values[3].el;
  let x_47 : f32 = x_6.x_GLF_uniform_float_values[4].el;
  M1 = mat2x2<f32>(vec2<f32>(x_41, x_43), vec2<f32>(x_45, x_47));
  let x_52 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  a = x_52;
  let x_54 : i32 = x_10.x_GLF_uniform_int_values[1].el;
  c = x_54;
  loop {
    let x_59 : i32 = c;
    let x_61 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    if ((x_59 < x_61)) {
    } else {
      break;
    }
    let x_65 : i32 = x_10.x_GLF_uniform_int_values[2].el;
    let x_66 : i32 = c;
    let x_70 : f32 = M1[x_65][clamp(~(x_66), 0, 1)];
    let x_71 : f32 = a;
    a = (x_71 + x_70);

    continuing {
      let x_73 : i32 = c;
      c = (x_73 + 1);
    }
  }
  let x_75 : f32 = a;
  let x_77 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  if ((x_75 == x_77)) {
    let x_83 : i32 = x_10.x_GLF_uniform_int_values[2].el;
    let x_86 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_89 : i32 = x_10.x_GLF_uniform_int_values[1].el;
    let x_92 : i32 = x_10.x_GLF_uniform_int_values[2].el;
    x_GLF_color = vec4<f32>(f32(x_83), f32(x_86), f32(x_89), f32(x_92));
  } else {
    let x_96 : i32 = x_10.x_GLF_uniform_int_values[2].el;
    let x_97 : f32 = f32(x_96);
    x_GLF_color = vec4<f32>(x_97, x_97, x_97, x_97);
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
