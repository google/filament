#include <vector>

#include <emscripten/bind.h>

#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"

using namespace emscripten;

///
/// Simple C++ wrapper class for Emscripten
///
class EXRLoader {
 public:
  ///
  /// `binary` is the buffer for EXR binary(e.g. buffer read by fs.readFileSync)
  /// std::string can be used as UInt8Array in JS layer.
  ///
  EXRLoader(const std::string &binary) {
    const float *ptr = reinterpret_cast<const float *>(binary.data());

    float *rgba = nullptr;
    width_ = -1;
    height_ = -1;
    const char *err = nullptr;

    error_.clear();

    result_ = LoadEXRFromMemory(
        &rgba, &width_, &height_,
        reinterpret_cast<const unsigned char *>(binary.data()), binary.size(),
        &err);

    if (TINYEXR_SUCCESS == result_) {
      image_.resize(size_t(width_ * height_ * 4));
      memcpy(image_.data(), rgba, sizeof(float) * size_t(width_ * height_ * 4));
      free(rgba);
    } else {
      if (err) {
        error_ = std::string(err);
      }
    }
  }
  ~EXRLoader() {}

  // Return as memory views
  emscripten::val getBytes() const {
    return emscripten::val(
        emscripten::typed_memory_view(image_.size(), image_.data()));
  }

  bool ok() const { return (TINYEXR_SUCCESS == result_); }

  const std::string error() const { return error_; }

  int width() const { return width_; }

  int height() const { return height_; }

 private:
  std::vector<float> image_;  // RGBA
  int width_;
  int height_;
  int result_;
  std::string error_;
};

// Register STL
EMSCRIPTEN_BINDINGS(stl_wrappters) { register_vector<float>("VectorFloat"); }

EMSCRIPTEN_BINDINGS(tinyexr_module) {
  class_<EXRLoader>("EXRLoader")
      .constructor<const std::string &>()
      .function("getBytes", &EXRLoader::getBytes)
      .function("ok", &EXRLoader::ok)
      .function("error", &EXRLoader::error)
      .function("width", &EXRLoader::width)
      .function("height", &EXRLoader::height);
}
