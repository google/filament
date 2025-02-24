struct buf0 {
  resolution : vec2<f32>,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var indexable : array<vec4<f32>, 16u>;
  var x_69 : i32;
  var x_69_phi : i32;
  var x_72_phi : i32;
  let x_55 : vec4<f32> = gl_FragCoord;
  let x_58 : vec2<f32> = x_6.resolution;
  let x_59 : vec2<f32> = (vec2<f32>(x_55.x, x_55.y) / x_58);
  let x_67 : i32 = (i32((x_59.x * 10.0)) + (i32((x_59.y * 10.0)) * 10));
  x_69_phi = 100;
  x_72_phi = 0;
  loop {
    var x_70 : i32;
    var x_73 : i32;
    x_69 = x_69_phi;
    let x_72 : i32 = x_72_phi;
    if ((x_72 < x_67)) {
    } else {
      break;
    }

    continuing {
      x_70 = (((4 * bitcast<i32>(x_69)) * (1000 - bitcast<i32>(x_69))) / 1000);
      x_73 = (x_72 + 1);
      x_69_phi = x_70;
      x_72_phi = x_73;
    }
  }
  indexable = array<vec4<f32>, 16u>(vec4<f32>(0.0, 0.0, 0.0, 1.0), vec4<f32>(0.5, 0.0, 0.0, 1.0), vec4<f32>(0.0, 0.5, 0.0, 1.0), vec4<f32>(0.5, 0.5, 0.0, 1.0), vec4<f32>(0.0, 0.0, 0.5, 1.0), vec4<f32>(0.5, 0.0, 0.5, 1.0), vec4<f32>(0.0, 0.5, 0.5, 1.0), vec4<f32>(0.5, 0.5, 0.5, 1.0), vec4<f32>(0.0, 0.0, 0.0, 1.0), vec4<f32>(1.0, 0.0, 0.0, 1.0), vec4<f32>(0.0, 1.0, 0.0, 1.0), vec4<f32>(1.0, 1.0, 0.0, 1.0), vec4<f32>(0.0, 0.0, 1.0, 1.0), vec4<f32>(1.0, 0.0, 1.0, 1.0), vec4<f32>(0.0, 1.0, 1.0, 1.0), vec4<f32>(1.0, 1.0, 1.0, 1.0));
  let x_80 : vec2<f32> = vec2<f32>(vec4<f32>(0.0, 1.0, 0.0, 1.0).y, vec4<f32>(0.5, 0.0, 0.5, 1.0).x);
  let x_82 : vec4<f32> = indexable[bitcast<i32>((x_69 % 16))];
  x_GLF_color = x_82;
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
