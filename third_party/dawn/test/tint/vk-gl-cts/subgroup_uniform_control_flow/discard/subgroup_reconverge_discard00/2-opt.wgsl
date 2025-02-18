var<private> gl_FragCoord : vec4<f32>;

var<private> expect : i32;

fn main_1() {
  var inbounds : bool;
  var x_31 : bool;
  var x_32_phi : bool;
  let x_24 : f32 = gl_FragCoord.x;
  let x_25 : bool = (x_24 < 128.0);
  x_32_phi = x_25;
  if (!(x_25)) {
    let x_30 : f32 = gl_FragCoord.y;
    x_31 = (x_30 < 128.0);
    x_32_phi = x_31;
  }
  let x_32 : bool = x_32_phi;
  inbounds = x_32;
  let x_33 : bool = inbounds;
  expect = select(-1, 1, x_33);
  return;
}

struct main_out {
  @location(0) @interpolate(flat)
  expect_1 : i32,
}

@fragment
fn main(@builtin(position) gl_FragCoord_param : vec4<f32>) -> main_out {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  return main_out(expect);
}
