struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 3u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : f32,
}

alias Arr_1 = array<strided_arr_1, 1u>;

struct buf1 {
  x_GLF_uniform_float_values : Arr_1,
}

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(1) var<uniform> x_9 : buf1;

fn main_1() {
  var i : i32;
  let x_34 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_35 : f32 = f32(x_34);
  x_GLF_color = vec4<f32>(x_35, x_35, x_35, x_35);
  let x_38 : f32 = gl_FragCoord.y;
  let x_40 : f32 = x_9.x_GLF_uniform_float_values[0].el;
  i = (1 << bitcast<u32>(select(1, 2, (x_38 >= x_40))));
  loop {
    var x_57 : bool;
    var x_58_phi : bool;
    let x_48 : i32 = i;
    let x_50 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_51 : bool = (x_48 != x_50);
    x_58_phi = x_51;
    if (x_51) {
      let x_54 : i32 = i;
      let x_56 : i32 = x_6.x_GLF_uniform_int_values[1].el;
      x_57 = (x_54 < x_56);
      x_58_phi = x_57;
    }
    let x_58 : bool = x_58_phi;
    if (x_58) {
    } else {
      break;
    }
    let x_61 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    let x_64 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    let x_67 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    let x_70 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    x_GLF_color = vec4<f32>(f32(x_61), f32(x_64), f32(x_67), f32(x_70));

    continuing {
      let x_73 : i32 = i;
      i = (x_73 + 1);
    }
  }
  return;
}

struct main_out {
  @location(0)
  x_GLF_color_1 : vec4<f32>,
}

@fragment
fn main(@builtin(position) gl_FragCoord_param : vec4<f32>) -> main_out {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  return main_out(x_GLF_color);
}
