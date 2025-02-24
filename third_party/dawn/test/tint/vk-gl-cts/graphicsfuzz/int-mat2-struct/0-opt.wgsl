struct S {
  f1 : i32,
  f2 : mat2x2<f32>,
}

var<private> gl_FragCoord : vec4<f32>;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var x_41 : mat2x2<f32>;
  var x_6 : i32;
  var x_42 : mat2x2<f32>;
  var x_49_phi : mat2x2<f32>;
  let x_44 : f32 = gl_FragCoord.x;
  if ((x_44 < 0.0)) {
    x_42 = mat2x2<f32>(vec2<f32>(1.0, 2.0), vec2<f32>(3.0, 4.0));
    x_49_phi = mat2x2<f32>(vec2<f32>(1.0, 2.0), vec2<f32>(3.0, 4.0));
  } else {
    x_42 = mat2x2<f32>(vec2<f32>(0.5, -0.5), vec2<f32>(-0.5, 0.5));
    x_49_phi = mat2x2<f32>(vec2<f32>(0.5, -0.5), vec2<f32>(-0.5, 0.5));
  }
  let x_49 : mat2x2<f32> = x_49_phi;
  let x_51 : S = S(1, transpose(x_49));
  let x_52 : i32 = x_51.f1;
  x_6 = x_52;
  x_41 = x_51.f2;
  let x_56 : mat2x2<f32> = x_41;
  let x_59 : mat2x2<f32> = x_41;
  let x_63 : mat2x2<f32> = x_41;
  let x_66 : mat2x2<f32> = x_41;
  x_GLF_color = vec4<f32>(f32(x_52), (x_56[0u].x + x_59[1u].x), (x_63[0u].y + x_66[1u].y), f32(x_52));
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
