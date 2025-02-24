struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 5u>;

struct buf1 {
  x_GLF_uniform_int_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : f32,
}

alias Arr_1 = array<strided_arr_1, 2u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr_1,
}

@group(0) @binding(1) var<uniform> x_6 : buf1;

@group(0) @binding(0) var<uniform> x_9 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var v : vec4<f32>;
  var i : i32;
  let x_40 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_43 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_46 : i32 = x_6.x_GLF_uniform_int_values[3].el;
  let x_49 : i32 = x_6.x_GLF_uniform_int_values[0].el;
  v = vec4<f32>(f32(x_40), f32(x_43), f32(x_46), f32(x_49));
  let x_53 : i32 = x_6.x_GLF_uniform_int_values[4].el;
  i = x_53;
  loop {
    let x_58 : i32 = i;
    let x_60 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    if ((x_58 < x_60)) {
    } else {
      break;
    }
    let x_63 : vec4<f32> = v;
    let x_64 : vec4<f32> = v;
    let x_65 : vec4<f32> = v;
    let x_66 : vec4<f32> = v;
    let x_88 : i32 = i;
    let x_92 : f32 = x_9.x_GLF_uniform_float_values[0].el;
    if ((mat4x4<f32>(vec4<f32>(x_63.x, x_63.y, x_63.z, x_63.w), vec4<f32>(x_64.x, x_64.y, x_64.z, x_64.w), vec4<f32>(x_65.x, x_65.y, x_65.z, x_65.w), vec4<f32>(x_66.x, x_66.y, x_66.z, x_66.w))[0u][x_88] > x_92)) {
      let x_96 : i32 = i;
      let x_97 : vec4<f32> = v;
      let x_99 : f32 = x_9.x_GLF_uniform_float_values[1].el;
      let x_102 : f32 = x_9.x_GLF_uniform_float_values[0].el;
      let x_106 : i32 = x_6.x_GLF_uniform_int_values[1].el;
      v[x_96] = clamp(x_97, vec4<f32>(x_99, x_99, x_99, x_99), vec4<f32>(x_102, x_102, x_102, x_102))[x_106];
    }

    continuing {
      let x_109 : i32 = i;
      i = (x_109 + 1);
    }
  }
  let x_111 : vec4<f32> = v;
  let x_113 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_114 : f32 = f32(x_113);
  if (all((x_111 == vec4<f32>(x_114, x_114, x_114, x_114)))) {
    let x_122 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_125 : i32 = x_6.x_GLF_uniform_int_values[4].el;
    let x_128 : i32 = x_6.x_GLF_uniform_int_values[4].el;
    let x_131 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    x_GLF_color = vec4<f32>(f32(x_122), f32(x_125), f32(x_128), f32(x_131));
  } else {
    let x_135 : i32 = x_6.x_GLF_uniform_int_values[4].el;
    let x_136 : f32 = f32(x_135);
    x_GLF_color = vec4<f32>(x_136, x_136, x_136, x_136);
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
