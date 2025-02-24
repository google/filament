struct S {
  a : i32,
  b : i32,
  c : i32,
}

struct buf0 {
  one : i32,
}

@group(0) @binding(0) var<uniform> x_10 : buf0;

var<private> x_GLF_color : vec4<f32>;

fn main_1() {
  var x_43 : i32;
  var x_44 : bool = false;
  var arr : array<S, 2u>;
  var param : S;
  var param_1 : i32;
  loop {
    var x_50 : i32;
    x_50 = x_10.one;
    arr[x_50].a = 2;
    let x_53 : i32 = arr[1].a;
    if ((x_53 < 1)) {
      x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
      x_44 = true;
      break;
    } else {
      let x_60 : S = arr[1];
      param = x_60;
      param_1 = (2 + bitcast<i32>(x_50));
      let x_61 : i32 = param_1;
      let x_63 : S = param;
      var x_64_1 : S = x_63;
      x_64_1.a = x_61;
      let x_64 : S = x_64_1;
      param = x_64;
      let x_65 : S = param;
      if ((x_65.a == 2)) {
        let x_70 : S = param;
        var x_71_1 : S = x_70;
        x_71_1.a = 9;
        let x_71 : S = x_71_1;
        param = x_71;
      }
      let x_72 : i32 = param_1;
      let x_75 : S = param;
      var x_76_1 : S = x_75;
      x_76_1.b = (x_72 + 1);
      let x_76 : S = x_76_1;
      param = x_76;
      let x_77 : i32 = param_1;
      let x_80 : S = param;
      var x_81_1 : S = x_80;
      x_81_1.c = (x_77 + 2);
      let x_81 : S = x_81_1;
      param = x_81;
      let x_82 : S = param;
      if ((x_82.b == 2)) {
        let x_87 : S = param;
        var x_88_1 : S = x_87;
        x_88_1.b = 7;
        let x_88 : S = x_88_1;
        param = x_88;
      }
      let x_89 : S = param;
      let x_91 : S = param;
      let x_94 : S = param;
      x_43 = ((x_89.a + x_91.b) + x_94.c);
      let x_97 : i32 = x_43;
      if ((x_97 == 12)) {
        x_GLF_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
      } else {
        x_GLF_color = vec4<f32>(0.0, 0.0, 0.0, 0.0);
      }
    }
    x_44 = true;
    break;
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

fn func_struct_S_i1_i1_i11_i1_(s : ptr<function, S>, x : ptr<function, i32>) -> i32 {
  let x_103 : i32 = *(x);
  (*(s)).a = x_103;
  let x_105 : i32 = (*(s)).a;
  if ((x_105 == 2)) {
    (*(s)).a = 9;
  }
  let x_109 : i32 = *(x);
  (*(s)).b = (x_109 + 1);
  let x_112 : i32 = *(x);
  (*(s)).c = (x_112 + 2);
  let x_115 : i32 = (*(s)).b;
  if ((x_115 == 2)) {
    (*(s)).b = 7;
  }
  let x_119 : i32 = (*(s)).a;
  let x_120 : i32 = (*(s)).b;
  let x_122 : i32 = (*(s)).c;
  return ((x_119 + x_120) + x_122);
}
