struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 5u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

@group(0) @binding(0) var<uniform> x_8 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn func_i1_(x : ptr<function, i32>) -> i32 {
  var a : i32;
  var i : i32;
  var indexable : array<i32, 4u>;
  let x_72 : i32 = x_8.x_GLF_uniform_int_values[0].el;
  a = x_72;
  let x_74 : i32 = x_8.x_GLF_uniform_int_values[0].el;
  i = x_74;
  loop {
    let x_79 : i32 = i;
    let x_81 : i32 = x_8.x_GLF_uniform_int_values[1].el;
    if ((x_79 < x_81)) {
    } else {
      break;
    }
    let x_85 : i32 = x_8.x_GLF_uniform_int_values[3].el;
    let x_87 : i32 = x_8.x_GLF_uniform_int_values[3].el;
    let x_89 : i32 = x_8.x_GLF_uniform_int_values[3].el;
    let x_91 : i32 = x_8.x_GLF_uniform_int_values[3].el;
    let x_93 : i32 = a;
    indexable = array<i32, 4u>(x_85, x_87, x_89, x_91);
    let x_95 : i32 = indexable[x_93];
    let x_96 : i32 = *(x);
    if ((x_95 > x_96)) {
      if (true) {
        let x_105 : i32 = x_8.x_GLF_uniform_int_values[3].el;
        return x_105;
      } else {
        let x_107 : i32 = x_8.x_GLF_uniform_int_values[3].el;
        a = x_107;
      }
    } else {
      if (true) {
        let x_111 : i32 = x_8.x_GLF_uniform_int_values[4].el;
        return x_111;
      }
    }

    continuing {
      let x_112 : i32 = i;
      i = (x_112 + 1);
    }
  }
  let x_115 : i32 = x_8.x_GLF_uniform_int_values[0].el;
  return x_115;
}

fn main_1() {
  var a_1 : i32;
  var param : i32;
  var param_1 : i32;
  let x_39 : i32 = x_8.x_GLF_uniform_int_values[0].el;
  param = x_39;
  let x_40 : i32 = func_i1_(&(param));
  a_1 = x_40;
  let x_42 : i32 = x_8.x_GLF_uniform_int_values[3].el;
  param_1 = x_42;
  let x_43 : i32 = func_i1_(&(param_1));
  let x_44 : i32 = a_1;
  a_1 = (x_44 + x_43);
  let x_46 : i32 = a_1;
  let x_48 : i32 = x_8.x_GLF_uniform_int_values[2].el;
  if ((x_46 == x_48)) {
    let x_54 : i32 = x_8.x_GLF_uniform_int_values[3].el;
    let x_57 : i32 = x_8.x_GLF_uniform_int_values[0].el;
    let x_60 : i32 = x_8.x_GLF_uniform_int_values[0].el;
    let x_63 : i32 = x_8.x_GLF_uniform_int_values[3].el;
    x_GLF_color = vec4<f32>(f32(x_54), f32(x_57), f32(x_60), f32(x_63));
  } else {
    let x_67 : i32 = x_8.x_GLF_uniform_int_values[0].el;
    let x_68 : f32 = f32(x_67);
    x_GLF_color = vec4<f32>(x_68, x_68, x_68, x_68);
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
