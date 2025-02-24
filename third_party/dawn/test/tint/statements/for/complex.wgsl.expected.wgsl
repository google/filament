fn some_loop_body() {
}

fn f() {
  var j : i32;
  for(var i : i32 = 0; ((i < 5) && (j < 10)); i = (i + 1)) {
    some_loop_body();
    j = (i * 30);
  }
}
