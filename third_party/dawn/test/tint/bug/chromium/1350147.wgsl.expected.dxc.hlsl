[numthreads(1, 1, 1)]
void unused_entry_point() {
  return;
}

void original_clusterfuzz_code() {
}

void more_tests_that_would_fail() {
  {
    float a = 1.47112762928009033203f;
    float b = 0.09966865181922912598f;
  }
  {
    float a = 2.5f;
    float b = 2.5f;
  }
  {
  }
}
