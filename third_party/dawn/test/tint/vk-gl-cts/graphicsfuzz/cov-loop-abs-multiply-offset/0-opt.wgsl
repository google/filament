struct strided_arr {
  @size(16)
  el : f32,
}

alias Arr = array<strided_arr, 4u>;

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
  var f : f32;
  var i : i32;
  var x_66 : bool;
  var x_67_phi : bool;
  let x_34 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  f = x_34;
  let x_36 : i32 = x_9.x_GLF_uniform_int_values[1].el;
  i = x_36;
  loop {
    let x_41 : i32 = i;
    let x_43 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    if ((x_41 < x_43)) {
    } else {
      break;
    }
    let x_47 : f32 = x_6.x_GLF_uniform_float_values[3].el;
    let x_49 : f32 = f;
    let x_53 : f32 = x_6.x_GLF_uniform_float_values[0].el;
    f = (abs((-(x_47) * x_49)) + x_53);

    continuing {
      let x_55 : i32 = i;
      i = (x_55 + 1);
    }
  }
  let x_57 : f32 = f;
  let x_59 : f32 = x_6.x_GLF_uniform_float_values[1].el;
  let x_60 : bool = (x_57 > x_59);
  x_67_phi = x_60;
  if (x_60) {
    let x_63 : f32 = f;
    let x_65 : f32 = x_6.x_GLF_uniform_float_values[2].el;
    x_66 = (x_63 < x_65);
    x_67_phi = x_66;
  }
  let x_67 : bool = x_67_phi;
  if (x_67) {
    let x_72 : i32 = x_9.x_GLF_uniform_int_values[2].el;
    let x_75 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    let x_78 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    let x_81 : i32 = x_9.x_GLF_uniform_int_values[2].el;
    x_GLF_color = vec4<f32>(f32(x_72), f32(x_75), f32(x_78), f32(x_81));
  } else {
    let x_85 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    let x_86 : f32 = f32(x_85);
    x_GLF_color = vec4<f32>(x_86, x_86, x_86, x_86);
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
