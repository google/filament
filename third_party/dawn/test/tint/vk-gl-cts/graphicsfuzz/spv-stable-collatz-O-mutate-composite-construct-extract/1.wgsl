struct buf0 {
  resolution : vec2<f32>,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var indexable : array<vec4<f32>, 16u>;
  var x_71 : i32;
  var x_71_phi : i32;
  var x_74_phi : i32;
  let x_54 : vec4<f32> = gl_FragCoord;
  let x_55 : vec2<f32> = vec2<f32>(x_54.x, x_54.y);
  let x_58 : vec2<f32> = x_6.resolution;
  let x_61 : vec2<f32> = ((x_55 / x_58) * 8.0);
  let x_62 : vec2<f32> = floor(x_61);
  let x_69 : i32 = ((i32(x_62.x) * 8) + i32(x_62.y));
  x_71_phi = 0;
  x_74_phi = x_69;
  loop {
    var x_85 : i32;
    var x_86 : i32;
    var x_75_phi : i32;
    x_71 = x_71_phi;
    let x_74 : i32 = x_74_phi;
    if ((x_74 > 1)) {
    } else {
      break;
    }
    if (((x_74 & 1) == 1)) {
      x_85 = ((3 * x_74) + 1);
      x_75_phi = x_85;
    } else {
      x_86 = (x_74 / 2);
      x_75_phi = x_86;
    }
    let x_75 : i32 = x_75_phi;

    continuing {
      x_71_phi = bitcast<i32>((x_71 + bitcast<i32>(1)));
      x_74_phi = x_75;
    }
  }
  indexable = array<vec4<f32>, 16u>(vec4<f32>(0.0, 0.0, 0.0, 1.0), vec4<f32>(0.5, 0.0, 0.0, 1.0), vec4<f32>(0.0, 0.5, 0.0, 1.0), vec4<f32>(0.5, 0.5, 0.0, 1.0), vec4<f32>(0.0, 0.0, 0.5, 1.0), vec4<f32>(0.5, 0.0, 0.5, 1.0), vec4<f32>(0.0, 0.5, 0.5, 1.0), vec4<f32>(0.5, 0.5, 0.5, 1.0), vec4<f32>(0.0, 0.0, 0.0, 1.0), vec4<f32>(1.0, 0.0, 0.0, 1.0), vec4<f32>(0.0, 1.0, 0.0, 1.0), vec4<f32>(1.0, 1.0, 0.0, 1.0), vec4<f32>(0.0, 0.0, 1.0, 1.0), vec4<f32>(1.0, 0.0, 1.0, 1.0), vec4<f32>(0.0, 1.0, 1.0, 1.0), vec4<f32>(1.0, 1.0, 1.0, 1.0));
  let x_88 : array<vec4<f32>, 16u> = indexable;
  indexable = array<vec4<f32>, 16u>(vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0));
  indexable = x_88;
  let x_89 : vec4<f32> = array<vec4<f32>, 16u>(vec4<f32>(0.0, 0.0, 0.5, 1.0), vec4<f32>(1.0, 0.0, 1.0, 1.0), vec4<f32>(0.0, 0.5, 0.5, 1.0), vec4<f32>(0.0, 1.0, 1.0, 1.0), vec4<f32>(1.0, 1.0, 1.0, 1.0), vec4<f32>(0.0, 0.5, 0.5, 1.0), vec4<f32>(0.0, 0.5, 0.5, 1.0), vec4<f32>(0.5, 0.0, 0.5, 1.0), vec4<f32>(1.0, 1.0, 0.0, 1.0), x_54, vec4<f32>(0.0, 0.0, 0.5, 1.0), vec4<f32>(0.5, 0.5, 0.0, 1.0), vec4<f32>(0.0, 0.5, 0.0, 1.0), vec4<f32>(0.5, 0.0, 0.0, 1.0), vec4<f32>(0.0, 1.0, 0.0, 1.0), vec4<f32>(1.0, 0.0, 0.0, 1.0))[1u];
  let x_90 : array<vec4<f32>, 16u> = array<vec4<f32>, 16u>(vec4<f32>(0.5, 0.0, 0.5, 1.0), vec4<f32>(0.5, 0.0, 0.5, 1.0), vec4<f32>(0.0, 1.0, 0.0, 1.0), vec4<f32>(0.0, 0.0, 0.0, 1.0), vec4<f32>(0.5, 0.0, 0.5, 1.0), vec4<f32>(0.5, 0.5, 0.5, 1.0), vec4<f32>(1.0, 0.0, 1.0, 1.0), vec4<f32>(0.0, 0.0, 0.0, 1.0), vec4<f32>(0.0, 1.0, 0.0, 1.0), vec4<f32>(0.0, 8.0, x_55), vec4<f32>(0.0, 0.0, 0.5, 1.0), vec4<f32>(0.0, 0.5, 0.5, 1.0), vec4<f32>(0.5, 0.0, 0.5, 1.0), vec4<f32>(1.0, 1.0, 0.0, 1.0), vec4<f32>(1.0, 0.0, 1.0, 1.0), vec4<f32>(x_61, 0.5, 1.0));
  let x_92 : vec4<f32> = indexable[bitcast<i32>((x_71 % 16))];
  x_GLF_color = x_92;
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
