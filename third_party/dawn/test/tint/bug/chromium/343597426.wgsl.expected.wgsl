fn foo(a : bool, b : bool, c : bool, d : bool, e : bool) {
  if (a) {
    if (b) {
      return;
    }
    if (c) {
      if (d) {
        return;
      }
      if (e) {
      }
    }
  }
}
