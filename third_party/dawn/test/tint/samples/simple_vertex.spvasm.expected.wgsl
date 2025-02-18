var<private> gl_Position : vec4f;

fn main_1() {
  gl_Position = vec4f();
  return;
}

struct main_out {
  @builtin(position)
  gl_Position : vec4f,
}

@vertex
fn main() -> main_out {
  main_1();
  return main_out(gl_Position);
}
