// Copyright 2019 The Marl Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// This is an example application that uses Marl to parallelize the calculation
// of a Julia fractal.

#include "marl/defer.h"
#include "marl/scheduler.h"
#include "marl/thread.h"
#include "marl/waitgroup.h"

#include <fstream>

#include <math.h>
#include <stdint.h>

// A color formed from a red, green and blue component.
template <typename T>
struct Color {
  T r, g, b;

  inline Color<T>& operator+=(const Color<T>& rhs) {
    r += rhs.r;
    g += rhs.g;
    b += rhs.b;
    return *this;
  }

  inline Color<T>& operator/=(T rhs) {
    r /= rhs;
    g /= rhs;
    b /= rhs;
    return *this;
  }
};

// colorize returns a 'rainbow-color' for the scalar v.
inline Color<float> colorize(float v) {
  constexpr float PI = 3.141592653589793f;
  constexpr float PI_2_THIRDS = 2.0f * PI / 3.0f;
  return Color<float>{
      0.5f + 0.5f * cosf(v + 0 * PI_2_THIRDS),
      0.5f + 0.5f * cosf(v + 1 * PI_2_THIRDS),
      0.5f + 0.5f * cosf(v + 2 * PI_2_THIRDS),
  };
}

// lerp returns the linear interpolation between min and max using the weight x.
inline float lerp(float x, float min, float max) {
  return min + x * (max - min);
}

// julia calculates the Julia-set fractal value for the given coordinate and
// constant. See https://en.wikipedia.org/wiki/Julia_set for more information.
Color<float> julia(float x, float y, float cx, float cy) {
  for (int i = 0; i < 1000; i++) {
    if (x * x + y * y > 4) {
      return colorize(sqrtf(static_cast<float>(i)));
    }

    auto xtemp = x * x - y * y;
    y = 2 * x * y + cy;
    x = xtemp + cx;
  }

  return {};
}

// writeBMP writes the given image as a bitmap to the given file, returning
// true on success and false on error.
bool writeBMP(const Color<uint8_t>* texels,
              int width,
              int height,
              const char* path) {
  auto file = fopen(path, "wb");
  if (!file) {
    fprintf(stderr, "Could not open file '%s'\n", path);
    return false;
  }
  defer(fclose(file));

  bool ok = true;
  auto put1 = [&](uint8_t val) { ok = ok && fwrite(&val, 1, 1, file) == 1; };
  auto put2 = [&](uint16_t val) { put1(static_cast<uint8_t>(val));
				  put1(static_cast<uint8_t>(val >> 8)); };
  auto put4 = [&](uint32_t val) { put2(static_cast<uint16_t>(val));
				  put2(static_cast<uint16_t>(val >> 16)); };

  const uint32_t padding = -(3 * width) & 3U;   // in bytes
  const uint32_t stride = 3 * width + padding;  // in bytes
  const uint32_t offset = 54;

  // Bitmap file header
  put1('B');  // header field
  put1('M');
  put4(offset + stride * height);  // size in bytes
  put4(0);                         // reserved
  put4(offset);

  // BITMAPINFOHEADER
  put4(40);      // size of header in bytes
  put4(width);   // width in pixels
  put4(height);  // height in pixels
  put2(1);       // number of color planes
  put2(24);      // bits per pixel
  put4(0);       // compression scheme (none)
  put4(0);       // size
  put4(72);      // horizontal resolution
  put4(72);      // vertical resolution
  put4(0);       // color pallete size
  put4(0);       // 'important colors' count

  for (int y = height - 1; y >= 0; y--) {
    for (int x = 0; x < width; x++) {
      auto& texel = texels[x + y * width];
      put1(texel.b);
      put1(texel.g);
      put1(texel.r);
    }
    for (uint32_t i = 0; i < padding; i++) {
      put1(0);
    }
  }

  return ok;
}

// Constants used for rendering the fractal.
constexpr uint32_t imageWidth = 2048;
constexpr uint32_t imageHeight = 2048;
constexpr int samplesPerPixelW = 3;
constexpr int samplesPerPixelH = 3;
constexpr float windowMinX = -0.5f;
constexpr float windowMaxX = +0.5f;
constexpr float windowMinY = -0.5f;
constexpr float windowMaxY = +0.5f;
constexpr float cx = -0.8f;
constexpr float cy = 0.156f;

int main() {
  // Create a marl scheduler using the full number of logical cpus.
  // Bind this scheduler to the main thread so we can call marl::schedule()
  marl::Scheduler scheduler(marl::Scheduler::Config::allCores());
  scheduler.bind();
  defer(scheduler.unbind());  // unbind before destructing the scheduler.

  // Allocate the image.
  auto pixels = new Color<uint8_t>[imageWidth * imageHeight];
  defer(delete[] pixels);  // free memory before returning.

  // Create a wait group that will be used to synchronize the tasks.
  // The wait group is constructed with an initial count of imageHeight as
  // there will be a total of imageHeight tasks.
  marl::WaitGroup wg(imageHeight);

  // For each line of the image...
  for (uint32_t y = 0; y < imageHeight; y++) {
    // Schedule a task to calculate the image for this line.
    // These may run concurrently across hardware threads.
    marl::schedule([=] {
      // Before this task returns, decrement the wait group counter.
      // This is used to indicate that the task is done.
      defer(wg.done());

      for (uint32_t x = 0; x < imageWidth; x++) {
        // Calculate the fractal pixel color.
        Color<float> color = {};
        // Take a number of sub-pixel samples.
        for (int sy = 0; sy < samplesPerPixelH; sy++) {
          auto fy = float(y) + (sy / float(samplesPerPixelH));
          auto dy = float(fy) / float(imageHeight);
          for (int sx = 0; sx < samplesPerPixelW; sx++) {
            auto fx = float(x) + (sx / float(samplesPerPixelW));
            auto dx = float(fx) / float(imageWidth);
            color += julia(lerp(dx, windowMinX, windowMaxX),
                           lerp(dy, windowMinY, windowMaxY), cx, cy);
          }
        }
        // Average the color.
        color /= samplesPerPixelW * samplesPerPixelH;
        // Write the pixel out to the image buffer.
        pixels[x + y * imageWidth] = {static_cast<uint8_t>(color.r * 255),
                                      static_cast<uint8_t>(color.g * 255),
                                      static_cast<uint8_t>(color.b * 255)};
      }
    });
  }

  // Wait until all image lines have been calculated.
  wg.wait();

  // Write the image to "fractal.bmp".
  if (!writeBMP(pixels, imageWidth, imageHeight, "fractal.bmp")) {
    return 1;
  }

  // All done.
  return 0;
}
