struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 3u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var i : i32;
  var A : array<i32, 2u>;
  var a : i32;
  let x_30 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  i = x_30;
  loop {
    let x_35 : i32 = i;
    let x_37 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    if ((x_35 < x_37)) {
    } else {
      break;
    }
    let x_40 : i32 = i;
    let x_41 : i32 = i;
    A[x_40] = x_41;

    continuing {
      let x_43 : i32 = i;
      i = (x_43 + 1);
    }
  }
  let x_46 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_48 : i32 = A[x_46];
  let x_51 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_53 : i32 = A[x_51];
  a = min(~(x_48), ~(x_53));
  let x_57 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_58 : f32 = f32(x_57);
  x_GLF_color = vec4<f32>(x_58, x_58, x_58, x_58);
  let x_60 : i32 = a;
  let x_62 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  if ((x_60 == -(x_62))) {
    let x_68 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    let x_71 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_74 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_77 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    x_GLF_color = vec4<f32>(f32(x_68), f32(x_71), f32(x_74), f32(x_77));
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
