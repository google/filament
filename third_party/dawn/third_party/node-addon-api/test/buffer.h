#include <cstdint>
#include <cstdlib>

namespace test_buffer {

const size_t testLength = 4;
extern uint16_t testData[testLength];
extern int finalizeCount;

template <typename T>
void InitData(T* data, size_t length) {
  for (size_t i = 0; i < length; i++) {
    data[i] = static_cast<T>(i);
  }
}

template <typename T>
bool VerifyData(T* data, size_t length) {
  for (size_t i = 0; i < length; i++) {
    if (data[i] != static_cast<T>(i)) {
      return false;
    }
  }
  return true;
}
}  // namespace test_buffer
