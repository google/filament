struct VSOut {
  @builtin(position)
  pos : vec4<f32>,
}

fn foo(out : ptr<function, VSOut>) {
  var pos = vec4f(1, 2, 3, 4);
  (*(out)).pos = pos;
}

@vertex
fn main() -> VSOut {
  var out : VSOut;
  foo(&(out));
  return out;
}
