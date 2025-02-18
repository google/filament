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
  var b : i32;
  var i : i32;
  var i_1 : i32;
  var i_2 : i32;
  var indexable : array<i32, 2u>;
  let x_36 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  a = x_36;
  let x_38 : i32 = x_6.x_GLF_uniform_int_values[3].el;
  b = x_38;
  let x_40 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_41 : f32 = f32(x_40);
  x_GLF_color = vec4<f32>(x_41, x_41, x_41, x_41);
  let x_44 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  i = x_44;
  loop {
    let x_49 : i32 = i;
    let x_51 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    if ((x_49 < x_51)) {
    } else {
      break;
    }
    let x_54 : i32 = i;
    let x_56 : i32 = x_6.x_GLF_uniform_int_values[3].el;
    if ((x_54 > x_56)) {
      let x_60 : i32 = a;
      a = (x_60 + 1);
      if (false) {
        let x_65 : i32 = x_6.x_GLF_uniform_int_values[2].el;
        i_1 = x_65;
        loop {
          let x_70 : i32 = i_1;
          let x_72 : i32 = x_6.x_GLF_uniform_int_values[0].el;
          if ((x_70 < x_72)) {
          } else {
            break;
          }
          return;
        }
      }
    }

    continuing {
      let x_75 : i32 = i;
      i = (x_75 + 1);
    }
  }
  let x_78 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  i_2 = x_78;
  loop {
    let x_83 : i32 = i_2;
    let x_85 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    if ((x_83 < x_85)) {
    } else {
      break;
    }
    let x_89 : i32 = x_6.x_GLF_uniform_int_values[3].el;
    let x_91 : i32 = x_6.x_GLF_uniform_int_values[4].el;
    let x_93 : i32 = b;
    indexable = array<i32, 2u>(x_89, x_91);
    let x_95 : i32 = indexable[x_93];
    let x_96 : i32 = a;
    a = (x_96 + x_95);

    continuing {
      let x_98 : i32 = i_2;
      i_2 = (x_98 + 1);
    }
  }
  let x_100 : i32 = a;
  let x_102 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  if ((x_100 == x_102)) {
    let x_107 : i32 = x_6.x_GLF_uniform_int_values[3].el;
    let x_110 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    let x_113 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    let x_116 : i32 = x_6.x_GLF_uniform_int_values[3].el;
    x_GLF_color = vec4<f32>(f32(x_107), f32(x_110), f32(x_113), f32(x_116));
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
