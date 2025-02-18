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

alias Arr_1 = array<strided_arr_1, 2u>;

struct buf1 {
  x_GLF_uniform_int_values : Arr_1,
}

@group(0) @binding(0) var<uniform> x_6 : buf0;

@group(0) @binding(1) var<uniform> x_9 : buf1;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var arr : array<f32, 5u>;
  var i : i32;
  var j : i32;
  let x_38 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  let x_40 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  let x_42 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  let x_44 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  let x_46 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  arr = array<f32, 5u>(x_38, x_40, x_42, x_44, x_46);
  let x_49 : i32 = x_9.x_GLF_uniform_int_values[1].el;
  i = x_49;
  j = 0;
  loop {
    let x_54 : i32 = i;
    let x_56 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    if ((x_54 < x_56)) {
    } else {
      break;
    }
    let x_59 : i32 = j;
    if ((x_59 < -1)) {
      break;
    }
    let x_63 : i32 = j;
    let x_65 : f32 = arr[x_63];
    arr[x_63] = (x_65 + 1.0);

    continuing {
      let x_68 : i32 = i;
      i = (x_68 + 1);
      let x_70 : i32 = j;
      j = (x_70 + 1);
    }
  }
  let x_73 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  let x_75 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  let x_77 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  let x_79 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  x_GLF_color = vec4<f32>(x_73, x_75, x_77, x_79);
  let x_82 : i32 = x_9.x_GLF_uniform_int_values[1].el;
  i = x_82;
  loop {
    let x_87 : i32 = i;
    let x_89 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    if ((x_87 < x_89)) {
    } else {
      break;
    }
    let x_92 : i32 = i;
    let x_94 : f32 = arr[x_92];
    if (!((x_94 == 2.0))) {
      let x_99 : f32 = x_6.x_GLF_uniform_float_values[1].el;
      x_GLF_color = vec4<f32>(x_99, x_99, x_99, x_99);
    }

    continuing {
      let x_101 : i32 = i;
      i = (x_101 + 1);
    }
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
