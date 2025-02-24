struct S {
  data : i32,
}

struct buf0 {
  one : i32,
}

@group(0) @binding(0) var<uniform> x_11 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn func_struct_S_i11_i1_(s : ptr<function, S>, x : ptr<function, i32>) -> i32 {
  let x_17 : i32 = (*(s)).data;
  if ((x_17 == 1)) {
    let x_18 : i32 = *(x);
    let x_19 : i32 = (*(s)).data;
    return (x_18 + x_19);
  } else {
    let x_21 : i32 = *(x);
    return x_21;
  }
}

fn main_1() {
  var a : i32;
  var arr : array<S, 1u>;
  var i : i32;
  var param : S;
  var param_1 : i32;
  var param_2 : S;
  var param_3 : i32;
  a = 0;
  let x_22 : i32 = x_11.one;
  arr[0].data = x_22;
  i = 0;
  loop {
    let x_23 : i32 = i;
    let x_24 : i32 = x_11.one;
    if ((x_23 < (5 + x_24))) {
    } else {
      break;
    }
    let x_26 : i32 = i;
    if (((x_26 % 2) != 0)) {
      let x_74 : S = arr[0];
      param = x_74;
      let x_28 : i32 = i;
      param_1 = x_28;
      let x_29 : i32 = func_struct_S_i11_i1_(&(param), &(param_1));
      let x_75 : S = param;
      arr[0] = x_75;
      a = x_29;
    } else {
      let x_78 : S = arr[0];
      param_2 = x_78;
      param_3 = 1;
      let x_30 : i32 = func_struct_S_i11_i1_(&(param_2), &(param_3));
      let x_79 : S = param_2;
      arr[0] = x_79;
      a = x_30;
    }

    continuing {
      let x_31 : i32 = i;
      i = (x_31 + 1);
    }
  }
  let x_33 : i32 = a;
  if ((x_33 == 6)) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  } else {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
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
