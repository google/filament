// This is a simple loop that sums elements of an input array and
// returns the result.  It's here mainly because it's one of the
// simple examples guiding the early Subzero design.

int simple_loop(int *a, int n) {
  int sum = 0;
  for (int i = 0; i < n; ++i)
    sum += a[i];
  return sum;
}
