struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 4u>;

struct buf1 {
  x_GLF_uniform_int_values : Arr,
}

struct strided_arr_1 {
  @size(16)
  el : f32,
}

alias Arr_1 = array<strided_arr_1, 1u>;

struct buf0 {
  x_GLF_uniform_float_values : Arr_1,
}

@group(0) @binding(1) var<uniform> x_6 : buf1;

@group(0) @binding(0) var<uniform> x_10 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var m0 : mat4x4<f32>;
  var c : i32;
  var m1 : mat4x4<f32>;
  let x_40 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_41 : f32 = f32(x_40);
  m0 = mat4x4<f32>(vec4<f32>(x_41, 0.0, 0.0, 0.0), vec4<f32>(0.0, x_41, 0.0, 0.0), vec4<f32>(0.0, 0.0, x_41, 0.0), vec4<f32>(0.0, 0.0, 0.0, x_41));
  let x_48 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  c = x_48;
  loop {
    let x_53 : i32 = c;
    let x_55 : i32 = x_6.x_GLF_uniform_int_values[0].el;
    if ((x_53 < x_55)) {
    } else {
      break;
    }
    let x_58 : mat4x4<f32> = m0;
    m1 = x_58;
    let x_59 : i32 = c;
    let x_61 : i32 = x_6.x_GLF_uniform_int_values[3].el;
    let x_64 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    let x_66 : f32 = x_10.x_GLF_uniform_float_values[0].el;
    m1[(x_59 % x_61)][x_64] = x_66;
    let x_68 : i32 = c;
    let x_70 : i32 = x_6.x_GLF_uniform_int_values[3].el;
    let x_73 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    let x_75 : f32 = x_10.x_GLF_uniform_float_values[0].el;
    m0[(x_68 % x_70)][x_73] = x_75;

    continuing {
      let x_77 : i32 = c;
      c = (x_77 + 1);
    }
  }
  let x_79 : mat4x4<f32> = m0;
  let x_81 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_84 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_87 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_90 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_93 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_96 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_99 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_102 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_105 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_108 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_111 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_114 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_117 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_120 : i32 = x_6.x_GLF_uniform_int_values[2].el;
  let x_123 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_126 : i32 = x_6.x_GLF_uniform_int_values[1].el;
  let x_132 : mat4x4<f32> = mat4x4<f32>(vec4<f32>(f32(x_81), f32(x_84), f32(x_87), f32(x_90)), vec4<f32>(f32(x_93), f32(x_96), f32(x_99), f32(x_102)), vec4<f32>(f32(x_105), f32(x_108), f32(x_111), f32(x_114)), vec4<f32>(f32(x_117), f32(x_120), f32(x_123), f32(x_126)));
  if ((((all((x_79[0u] == x_132[0u])) & all((x_79[1u] == x_132[1u]))) & all((x_79[2u] == x_132[2u]))) & all((x_79[3u] == x_132[3u])))) {
    let x_156 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    let x_159 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_162 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_165 : i32 = x_6.x_GLF_uniform_int_values[2].el;
    x_GLF_color = vec4<f32>(f32(x_156), f32(x_159), f32(x_162), f32(x_165));
  } else {
    let x_169 : i32 = x_6.x_GLF_uniform_int_values[1].el;
    let x_170 : f32 = f32(x_169);
    x_GLF_color = vec4<f32>(x_170, x_170, x_170, x_170);
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
