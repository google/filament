var<private> gl_FragCoord : vec4<f32>;

var<private> out_data : i32;

fn main_1() {
  var x_is_odd : bool;
  var y_is_odd : bool;
  let x_24 : f32 = gl_FragCoord.x;
  x_is_odd = ((i32(x_24) & 1) == 1);
  let x_29 : f32 = gl_FragCoord.y;
  y_is_odd = ((i32(x_29) & 1) == 1);
  let x_33 : bool = x_is_odd;
  let x_34 : bool = y_is_odd;
  out_data = select(0, 1, (x_33 | x_34));
  return;
}

struct main_out {
  @location(0) @interpolate(flat)
  out_data_1 : i32,
}

@fragment
fn main(@builtin(position) gl_FragCoord_param : vec4<f32>) -> main_out {
  gl_FragCoord = gl_FragCoord_param;
  main_1();
  return main_out(out_data);
}
