// ======================================================================== //
// Copyright 2009-2019 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#pragma once

#include <string>
#include <vector>
#include <array>

namespace oidn {

  class ImageBuffer
  {
  private:
    std::vector<float> data;
    int width;
    int height;
    int channels;

  public:
    ImageBuffer()
      : width(0),
        height(0),
        channels(0) {}

    ImageBuffer(int width, int height, int channels)
      : data(width * height * channels),
        width(width),
        height(height),
        channels(channels) {}

    operator bool() const
    {
      return data.data() != nullptr;
    }

    const float& operator [](size_t i) const { return data[i]; }
    float& operator [](size_t i) { return data[i]; }

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    std::array<int, 2> getSize() const { return {width, height}; }
    int getChannels() const { return channels; }

    const float* getData() const { return data.data(); }
    float* getData() { return data.data(); }
    int getDataSize() { return int(data.size()); }
  };

  ImageBuffer loadImage(const std::string& filename);
  void saveImage(const std::string& filename, const ImageBuffer& image);

} // namespace oidn
