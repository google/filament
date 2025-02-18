struct buf0 {
  resolution : vec2<f32>,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var indexable : array<vec4<f32>, 16u>;
  var x_65 : i32;
  var x_65_phi : i32;
  var x_68_phi : i32;
  let x_51 : vec4<f32> = gl_FragCoord;
  let x_54 : vec2<f32> = x_6.resolution;
  let x_57 : vec2<f32> = floor(((vec2<f32>(x_51.x, x_51.y) / x_54) * 8.0));
  let x_63 : i32 = ((i32(x_57.x) * 8) + i32(x_57.y));
  x_65_phi = 0;
  x_68_phi = x_63;
  loop {
    var x_79 : i32;
    var x_80 : i32;
    var x_69_phi : i32;
    x_65 = x_65_phi;
    let x_68 : i32 = x_68_phi;
    if ((x_68 > 1)) {
    } else {
      break;
    }
    if (((x_68 & 1) == 1)) {
      x_79 = ((3 * x_68) + 1);
      x_69_phi = x_79;
    } else {
      x_80 = (x_68 / 2);
      x_69_phi = x_80;
    }
    let x_69 : i32 = x_69_phi;

    continuing {
      x_65_phi = bitcast<i32>((x_65 + bitcast<i32>(1)));
      x_68_phi = x_69;
    }
  }
  indexable = array<vec4<f32>, 16u>(vec4<f32>(0.0, 0.0, 0.0, 1.0), vec4<f32>(0.5, 0.0, 0.0, 1.0), vec4<f32>(0.0, 0.5, 0.0, 1.0), vec4<f32>(0.5, 0.5, 0.0, 1.0), vec4<f32>(0.0, 0.0, 0.5, 1.0), vec4<f32>(0.5, 0.0, 0.5, 1.0), vec4<f32>(0.0, 0.5, 0.5, 1.0), vec4<f32>(0.5, 0.5, 0.5, 1.0), vec4<f32>(0.0, 0.0, 0.0, 1.0), vec4<f32>(1.0, 0.0, 0.0, 1.0), vec4<f32>(0.0, 1.0, 0.0, 1.0), vec4<f32>(1.0, 1.0, 0.0, 1.0), vec4<f32>(0.0, 0.0, 1.0, 1.0), vec4<f32>(1.0, 0.0, 1.0, 1.0), vec4<f32>(0.0, 1.0, 1.0, 1.0), vec4<f32>(1.0, 1.0, 1.0, 1.0));
  let x_83 : vec4<f32> = indexable[bitcast<i32>((x_65 % 16))];
  x_GLF_color = x_83;
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
