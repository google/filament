struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 3u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

@group(0) @binding(0) var<uniform> x_7 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var arr : array<i32, 10u>;
  var a : i32;
  var i : i32;
  arr = array<i32, 10u>(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
  a = 0;
  let x_42 : i32 = x_7.x_GLF_uniform_int_values[1].el;
  let x_44 : i32 = arr[x_42];
  if ((x_44 == 2)) {
    let x_49 : i32 = x_7.x_GLF_uniform_int_values[2].el;
    i = x_49;
    loop {
      let x_54 : i32 = i;
      let x_56 : i32 = x_7.x_GLF_uniform_int_values[0].el;
      if ((x_54 < x_56)) {
      } else {
        break;
      }

      continuing {
        let x_59 : i32 = i;
        i = (x_59 + 1);
      }
    }
    let x_61 : i32 = a;
    a = (x_61 + 1);
  }
  let x_63 : i32 = a;
  let x_66 : i32 = x_7.x_GLF_uniform_int_values[2].el;
  if (((-1 % x_63) == x_66)) {
    let x_71 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    let x_75 : i32 = x_7.x_GLF_uniform_int_values[2].el;
    arr[vec2<i32>(x_71, x_71).y] = x_75;
  }
  let x_78 : i32 = x_7.x_GLF_uniform_int_values[1].el;
  let x_80 : i32 = arr[x_78];
  let x_82 : i32 = x_7.x_GLF_uniform_int_values[2].el;
  if ((x_80 == x_82)) {
    let x_88 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    let x_91 : i32 = x_7.x_GLF_uniform_int_values[2].el;
    let x_94 : i32 = x_7.x_GLF_uniform_int_values[2].el;
    let x_97 : i32 = x_7.x_GLF_uniform_int_values[1].el;
    x_GLF_color = vec4<f32>(f32(x_88), f32(x_91), f32(x_94), f32(x_97));
  } else {
    let x_101 : i32 = x_7.x_GLF_uniform_int_values[2].el;
    let x_102 : f32 = f32(x_101);
    x_GLF_color = vec4<f32>(x_102, x_102, x_102, x_102);
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
