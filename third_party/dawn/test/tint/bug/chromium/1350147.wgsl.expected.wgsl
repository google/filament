fn original_clusterfuzz_code() {
  _ = atan2(1, 0.10000000000000000555);
}

fn more_tests_that_would_fail() {
  {
    let a = atan2(1, 0.10000000000000000555);
    let b = atan2(0.10000000000000000555, 1);
  }
  {
    let a = (1 + 1.5);
    let b = (1.5 + 1);
  }
  {
    _ = atan2(1, 0.10000000000000000555);
    _ = atan2(0.10000000000000000555, 1);
  }
}
