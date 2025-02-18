struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 5u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var a : i32;
  var i : i32;
  var indexable : array<i32, 9u>;
  let x_38 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  a = x_38;
  let x_40 : i32 = x_6.x_GLF_uniform_int_values[3].el;
  i = x_40;
  loop {
    let x_45 : i32 = i;
    let x_47 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    if ((x_45 < x_47)) {
    } else {
      break;
    }
    let x_50 : i32 = i;
    let x_52 : i32 = x_6.x_GLF_uniform_int_values[4].el;
    indexable = array<i32, 9u>(1, 2, 3, 4, 5, 6, 7, 8, 9);
    let x_55 : i32 = indexable[(x_50 % x_52)];
    let x_56 : i32 = a;
    a = (x_56 + x_55);

    continuing {
      let x_58 : i32 = i;
      i = (x_58 + 1);
    }
  }
  let x_60 : i32 = a;
  let x_62 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  if ((x_60 == x_62)) {
    let x_68 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    let x_71 : i32 = x_6.x_GLF_uniform_int_values[3].el;
    let x_74 : i32 = x_6.x_GLF_uniform_int_values[3].el;
    let x_77 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    x_GLF_color = vec4<f32>(f32(x_68), f32(x_71), f32(x_74), f32(x_77));
  } else {
    let x_81 : i32 = x_6.x_GLF_uniform_int_values[3].el;
    let x_82 : f32 = f32(x_81);
    x_GLF_color = vec4<f32>(x_82, x_82, x_82, x_82);
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
