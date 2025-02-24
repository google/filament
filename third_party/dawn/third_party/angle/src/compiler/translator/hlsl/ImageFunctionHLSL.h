//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImageFunctionHLSL: Class for writing implementations of ESSL image functions into HLSL output.
//

#ifndef COMPILER_TRANSLATOR_HLSL_IMAGEFUNCTIONHLSL_H_
#define COMPILER_TRANSLATOR_HLSL_IMAGEFUNCTIONHLSL_H_

#include <set>

#include "GLSLANG/ShaderLang.h"
#include "compiler/translator/BaseTypes.h"
#include "compiler/translator/Common.h"
#include "compiler/translator/InfoSink.h"
#include "compiler/translator/Types.h"

namespace sh
{

class ImageFunctionHLSL final : angle::NonCopyable
{
  public:
    // Returns the name of the image function implementation to caller.
    // The name that's passed in is the name of the GLSL image function that it should implement.
    ImmutableString useImageFunction(const ImmutableString &name,
                                     const TBasicType &type,
                                     TLayoutImageInternalFormat imageInternalFormat,
                                     bool readonly);

    void imageFunctionHeader(TInfoSinkBase &out);
    const std::set<std::string> &getUsedImage2DFunctionNames() const
    {
        return mUsedImage2DFunctionNames;
    }

  private:
    struct ImageFunction
    {
        // See ESSL 3.10.4 section 8.12 for reference about what the different methods below do.
        enum class Method
        {
            SIZE,
            LOAD,
            STORE
        };

        enum class DataType
        {
            NONE,
            FLOAT4,
            UINT4,
            INT4,
            UNORM_FLOAT4,
            SNORM_FLOAT4
        };

        ImmutableString name() const;

        bool operator<(const ImageFunction &rhs) const;

        DataType getDataType(TLayoutImageInternalFormat format) const;

        const char *getReturnType() const;

        TBasicType image;
        TLayoutImageInternalFormat imageInternalFormat;
        bool readonly;
        Method method;
        DataType type;
    };

    static ImmutableString GetImageReference(TInfoSinkBase &out,
                                             const ImageFunctionHLSL::ImageFunction &imageFunction);
    static void OutputImageFunctionArgumentList(
        TInfoSinkBase &out,
        const ImageFunctionHLSL::ImageFunction &imageFunction);
    static void OutputImageSizeFunctionBody(TInfoSinkBase &out,
                                            const ImageFunctionHLSL::ImageFunction &imageFunction,
                                            const ImmutableString &imageReference);
    static void OutputImageLoadFunctionBody(TInfoSinkBase &out,
                                            const ImageFunctionHLSL::ImageFunction &imageFunction,
                                            const ImmutableString &imageReference);
    static void OutputImageStoreFunctionBody(TInfoSinkBase &out,
                                             const ImageFunctionHLSL::ImageFunction &imageFunction,
                                             const ImmutableString &imageReference);
    using ImageFunctionSet = std::set<ImageFunction>;
    ImageFunctionSet mUsesImage;
    std::set<std::string> mUsedImage2DFunctionNames;
};

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_HLSL_IMAGEFUNCTIONHLSL_H_
