fn some_loop_body() {
}

fn f() {
  for(var i : i32 = 0; (i < 5); i = (i + 1)) {
    some_loop_body();
  }
}
