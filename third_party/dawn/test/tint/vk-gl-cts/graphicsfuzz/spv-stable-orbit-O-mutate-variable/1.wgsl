struct buf0 {
  resolution : vec2<f32>,
}

var<private> gl_FragCoord : vec4<f32>;

@group(0) @binding(0) var<uniform> x_6 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var indexable : array<vec4<f32>, 16u>;
  var x_80 : vec2<i32>;
  var x_113 : vec2<i32>;
  var x_119 : i32;
  var x_80_phi : vec2<i32>;
  var x_83_phi : i32;
  var x_114_phi : vec2<i32>;
  var x_116_phi : vec2<i32>;
  let x_59 : vec4<f32> = gl_FragCoord;
  let x_62 : vec2<f32> = x_6.resolution;
  let x_63 : vec2<f32> = (vec2<f32>(x_59.x, x_59.y) / x_62);
  let x_66 : i32 = i32((x_63.x * 8.0));
  let x_69 : i32 = i32((x_63.y * 8.0));
  let x_78 : vec2<i32> = vec2<i32>(((((x_66 & 5) | (x_69 & 10)) * 8) + ((x_69 & 5) | (x_66 & 10))), 0);
  x_80_phi = x_78;
  x_83_phi = 0;
  loop {
    var x_94 : vec2<i32>;
    var x_102 : vec2<i32>;
    var x_84 : i32;
    var x_95_phi : vec2<i32>;
    var x_103_phi : vec2<i32>;
    x_80 = x_80_phi;
    let x_83 : i32 = x_83_phi;
    if ((x_83 < 100)) {
    } else {
      break;
    }
    x_95_phi = x_80;
    if ((x_80.x > 0)) {
      x_94 = x_80;
      x_94.y = (x_80.y - 1);
      x_95_phi = x_94;
    }
    let x_95 : vec2<i32> = x_95_phi;
    x_103_phi = x_95;
    if ((x_95.x < 0)) {
      x_102 = x_95;
      x_102.y = (x_95.y + 1);
      x_103_phi = x_102;
    }
    let x_103 : vec2<i32> = x_103_phi;
    var x_81_1 : vec2<i32> = x_103;
    x_81_1.x = (x_103.x + (x_103.y / 2));
    let x_81 : vec2<i32> = x_81_1;

    continuing {
      x_84 = (x_83 + 1);
      x_80_phi = x_81;
      x_83_phi = x_84;
    }
  }
  let x_108 : i32 = x_80.x;
  x_114_phi = x_80;
  if ((x_108 < 0)) {
    x_113 = vec2<i32>();
    x_113.x = -(x_108);
    x_114_phi = x_113;
  }
  let x_114 : vec2<i32> = x_114_phi;
  x_116_phi = x_114;
  loop {
    var x_117 : vec2<i32>;
    let x_116 : vec2<i32> = x_116_phi;
    x_119 = x_116.x;
    if ((x_119 > 15)) {
    } else {
      break;
    }

    continuing {
      x_117 = vec2<i32>();
      x_117.x = bitcast<i32>((x_119 - bitcast<i32>(16)));
      x_116_phi = x_117;
    }
  }
  indexable = array<vec4<f32>, 16u>(vec4<f32>(0.0, 0.0, 0.0, 1.0), vec4<f32>(0.5, 0.0, 0.0, 1.0), vec4<f32>(0.0, 0.5, 0.0, 1.0), vec4<f32>(0.5, 0.5, 0.0, 1.0), vec4<f32>(0.0, 0.0, 0.5, 1.0), vec4<f32>(0.5, 0.0, 0.5, 1.0), vec4<f32>(0.0, 0.5, 0.5, 1.0), vec4<f32>(0.5, 0.5, 0.5, 1.0), vec4<f32>(0.0, 0.0, 0.0, 1.0), vec4<f32>(1.0, 0.0, 0.0, 1.0), vec4<f32>(0.0, 1.0, 0.0, 1.0), vec4<f32>(1.0, 1.0, 0.0, 1.0), vec4<f32>(0.0, 0.0, 1.0, 1.0), vec4<f32>(1.0, 0.0, 1.0, 1.0), vec4<f32>(0.0, 1.0, 1.0, 1.0), vec4<f32>(1.0, 1.0, 1.0, 1.0));
  let x_123 : array<vec4<f32>, 16u> = indexable;
  indexable = array<vec4<f32>, 16u>(vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0), vec4<f32>(0.0, 0.0, 0.0, 0.0));
  indexable = x_123;
  let x_125 : vec4<f32> = indexable[x_119];
  x_GLF_color = x_125;
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
