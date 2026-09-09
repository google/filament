
#include <chrono>
#include <thread>

#include "benchmark/benchmark.h"
#include "output_test.h"

// BM_ScopedPause sleeps for 10ms in a ScopedPauseTiming block.
// The reported time should be much less than 10ms.
void BM_ScopedPause(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::ScopedPauseTiming pause(state);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    state.SetIterationTime(0.0);
  }
}
BENCHMARK(BM_ScopedPause)->UseManualTime()->Iterations(1);

void CheckResults(Results const& results) {
  // Check that the real time is much less than the 10ms sleep time.
  // Allow for up to 1ms of timing noise/overhead.
  CHECK_FLOAT_RESULT_VALUE(results, "real_time", LT, 1e6, 0.0);
}
CHECK_BENCHMARK_RESULTS("BM_ScopedPause", &CheckResults);

int main(int argc, char* argv[]) {
  benchmark::MaybeReenterWithoutASLR(argc, argv);
  RunOutputTests(argc, argv);
  return 0;
}
