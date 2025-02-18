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
  var a : f32;
  var i : i32;
  var b : f32;
  var c : f32;
  var d : f32;
  var x_67 : bool;
  var x_68_phi : bool;
  let x_37 : f32 = x_6.x_GLF_uniform_float_values[0].el;
  a = x_37;
  let x_39 : i32 = x_9.x_GLF_uniform_int_values[1].el;
  i = x_39;
  loop {
    let x_44 : i32 = i;
    let x_46 : i32 = x_9.x_GLF_uniform_int_values[0].el;
    if ((x_44 < x_46)) {
    } else {
      break;
    }
    let x_49 : f32 = a;
    b = x_49;
    let x_50 : f32 = b;
    c = x_50;
    let x_51 : f32 = c;
    d = asin(x_51);
    let x_54 : f32 = x_6.x_GLF_uniform_float_values[1].el;
    c = x_54;
    let x_55 : f32 = d;
    a = x_55;

    continuing {
      let x_56 : i32 = i;
      i = (x_56 + 1);
    }
  }
  let x_59 : f32 = x_6.x_GLF_uniform_float_values[2].el;
  let x_60 : f32 = a;
  let x_61 : bool = (x_59 < x_60);
  x_68_phi = x_61;
  if (x_61) {
    let x_64 : f32 = a;
    let x_66 : f32 = x_6.x_GLF_uniform_float_values[3].el;
    x_67 = (x_64 < x_66);
    x_68_phi = x_67;
  }
  let x_68 : bool = x_68_phi;
  if (x_68) {
    let x_73 : i32 = x_9.x_GLF_uniform_int_values[2].el;
    let x_76 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    let x_79 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    let x_82 : i32 = x_9.x_GLF_uniform_int_values[2].el;
    x_GLF_color = vec4<f32>(f32(x_73), f32(x_76), f32(x_79), f32(x_82));
  } else {
    let x_86 : i32 = x_9.x_GLF_uniform_int_values[1].el;
    let x_87 : f32 = f32(x_86);
    x_GLF_color = vec4<f32>(x_87, x_87, x_87, x_87);
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
