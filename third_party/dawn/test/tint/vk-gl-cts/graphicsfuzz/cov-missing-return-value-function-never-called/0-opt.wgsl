struct buf1 {
  one : u32,
}

struct strided_arr {
  @size(16)
  el : i32,
}

alias Arr = array<strided_arr, 1u>;

struct buf0 {
  x_GLF_uniform_int_values : Arr,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(1) var<uniform> x_8 : buf1;

var<private> x_GLF_color : vec4<f32>;

@group(0) @binding(0) var<uniform> x_10 : buf0;

fn func_() -> f32 {
  switch(1) {
    case 0: {
      return 1.0;
    }
    default: {
    }
  }
  return 0.0;
}

fn main_1() {
  var v : vec4<f32>;
  v = vec4<f32>(1.0, 1.0, 1.0, 1.0);
  let x_38 : f32 = gl_FragCoord.y;
  if ((x_38 < 0.0)) {
    let x_42 : f32 = func_();
    v = vec4<f32>(x_42, x_42, x_42, x_42);
  }
  let x_44 : vec4<f32> = v;
  if ((pack4x8unorm(x_44) == 1u)) {
    return;
  }
  let x_50 : u32 = x_8.one;
  if (((1u << x_50) == 2u)) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  } else {
    let x_57 : i32 = x_10.x_GLF_uniform_int_values[0].el;
    let x_58 : f32 = f32(x_57);
    x_GLF_color = vec4<f32>(x_58, x_58, x_58, x_58);
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
