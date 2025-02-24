/* crosstest.py --test=simple_loop.c --driver=simple_loop_main.c \
   --prefix=Subzero_ --output=simple_loop */

#include <stdio.h>

int simple_loop(int *a, int n);
int Subzero_simple_loop(int *a, int n);

int main(int argc, char *argv[]) {
  unsigned TotalTests = 0;
  unsigned Passes = 0;
  unsigned Failures = 0;
  int a[100];
  for (int i = 0; i < 100; ++i)
    a[i] = i * 2 - 100;
  for (int i = -2; i < 100; ++i) {
    ++TotalTests;
    int llc_result = simple_loop(a, i);
    int sz_result = Subzero_simple_loop(a, i);
    if (llc_result == sz_result) {
      ++Passes;
    } else {
      ++Failures;
      printf("Failure: i=%d, llc=%d, sz=%d\n", i, llc_result, sz_result);
    }
  }
  printf("TotalTests=%u Passes=%u Failures=%u\n", TotalTests, Passes, Failures);
  return Failures;
}
