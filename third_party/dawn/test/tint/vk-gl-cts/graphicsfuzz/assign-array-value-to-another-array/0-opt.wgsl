var<private> x_GLF_color : vec4<f32>;

fn func_i1_(x : ptr<function, i32>) {
  var a : i32;
  var data : array<i32, 9u>;
  var temp : array<i32, 2u>;
  var i : i32;
  var x_95 : bool;
  var x_96_phi : bool;
  a = 0;
  data[0] = 5;
  loop {
    let x_56 : i32 = a;
    let x_57 : i32 = *(x);
    if ((x_56 <= x_57)) {
    } else {
      break;
    }
    let x_60 : i32 = a;
    if ((x_60 <= 10)) {
      let x_64 : i32 = a;
      let x_66 : i32 = a;
      let x_69 : i32 = data[min(x_66, 0)];
      temp[min(x_64, 1)] = x_69;
      let x_71 : i32 = a;
      a = (x_71 + 1);
    }
  }
  i = 0;
  loop {
    let x_77 : i32 = i;
    if ((x_77 < 2)) {
    } else {
      break;
    }
    let x_80 : i32 = i;
    let x_82 : i32 = temp[0];
    let x_83 : i32 = i;
    data[x_80] = (x_82 + x_83);

    continuing {
      let x_86 : i32 = i;
      i = (x_86 + 1);
    }
  }
  let x_89 : i32 = data[0];
  let x_90 : bool = (x_89 == 5);
  x_96_phi = x_90;
  if (x_90) {
    let x_94 : i32 = data[1];
    x_95 = (x_94 == 6);
    x_96_phi = x_95;
  }
  let x_96 : bool = x_96_phi;
  if (x_96) {
    x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  } else {
    x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
  }
  return;
}

fn main_1() {
  var i_1 : i32;
  var param : i32;
  i_1 = 1;
  loop {
    let x_43 : i32 = i_1;
    if ((x_43 < 6)) {
    } else {
      break;
    }
    let x_46 : i32 = i_1;
    param = x_46;
    func_i1_(&(param));

    continuing {
      let x_48 : i32 = i_1;
      i_1 = (x_48 + 1);
    }
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
