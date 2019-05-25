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

#include "image.h"
#include "node.h"

namespace oidn {

  __forceinline float luminance(float r, float g, float b)
  {
    return 0.212671f * r + 0.715160f * g + 0.072169f * b;
  }

  // Color transfer function
  class TransferFunction
  {
  public:
    virtual ~TransferFunction() = default;

    virtual float forward(float y) const = 0;
    virtual float inverse(float x) const = 0;
  };

  // LDR transfer function: linear
  class LDRLinearTransferFunction : public TransferFunction
  {
  public:
    __forceinline float forward(float y) const override
    {
      return min(y, 1.f);
    }

    __forceinline float inverse(float x) const override
    {
      return min(x, 1.f);
    }
  };

  // LDR transfer function: sRGB curve
  class LDRTransferFunction : public TransferFunction
  {
  public:
    __forceinline float forward(float y) const override
    {
      return pow(min(y, 1.f), 1.f/2.2f);
    }

    __forceinline float inverse(float x) const override
    {
      return min(pow(x, 2.2f), 1.f);
    }
  };

  // HDR transfer function: log + sRGB curve
  // Compresses [0..65535] to [0..1]
  class HDRTransferFunction : public TransferFunction
  {
  private:
    float exposure;
    float rcpExposure;

  public:
    HDRTransferFunction(float exposure = 1.f)
    {
      setExposure(exposure);
    }

    void setExposure(float exposure)
    {
      this->exposure = exposure;
      this->rcpExposure = 1.f / exposure;
    }

    __forceinline float forward(float y) const override
    {
      y *= exposure;
      return pow(log2(y+1.f) * (1.f/16.f), 1.f/2.2f);
    }

    __forceinline float inverse(float x) const override
    {
      return (exp2(pow(x, 2.2f) * 16.f) - 1.f) * rcpExposure;
    }
  };

  // Autoexposure node
  class AutoexposureNode : public Node
  {
  private:
    Image color;
    std::shared_ptr<HDRTransferFunction> transferFunc;

  public:
    AutoexposureNode(const Image& color,
                     const std::shared_ptr<HDRTransferFunction>& transferFunc)
      : color(color),
        transferFunc(transferFunc)
    {}

    void execute(stream& sm) override
    {
      const float exposure = autoexposure(color);
      //printf("exposure = %f\n", exposure);
      transferFunc->setExposure(exposure);
    }

  private:
    static float autoexposure(const Image& color);
  };

} // namespace oidn
