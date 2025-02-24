//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImageFunctionHLSL: Class for writing implementations of ESSL image functions into HLSL output.
//

#include "compiler/translator/hlsl/ImageFunctionHLSL.h"
#include "compiler/translator/ImmutableStringBuilder.h"
#include "compiler/translator/hlsl/UtilsHLSL.h"

namespace sh
{

// static
ImmutableString ImageFunctionHLSL::GetImageReference(
    TInfoSinkBase &out,
    const ImageFunctionHLSL::ImageFunction &imageFunction)
{
    static const ImmutableString kImageIndexStr("[index]");
    if (imageFunction.readonly)
    {
        static const ImmutableString kReadonlyImagesStr("readonlyImages");
        ImmutableString suffix(
            TextureGroupSuffix(imageFunction.image, imageFunction.imageInternalFormat));
        out << "    const uint index = imageIndex - readonlyImageIndexOffset" << suffix.data()
            << ";\n";
        ImmutableStringBuilder imageRefBuilder(kReadonlyImagesStr.length() + suffix.length() +
                                               kImageIndexStr.length());
        imageRefBuilder << kReadonlyImagesStr << suffix << kImageIndexStr;
        return imageRefBuilder;
    }
    else
    {
        static const ImmutableString kImagesStr("images");
        ImmutableString suffix(
            RWTextureGroupSuffix(imageFunction.image, imageFunction.imageInternalFormat));
        out << "    const uint index = imageIndex - imageIndexOffset" << suffix.data() << ";\n";
        ImmutableStringBuilder imageRefBuilder(kImagesStr.length() + suffix.length() +
                                               kImageIndexStr.length());
        imageRefBuilder << kImagesStr << suffix << kImageIndexStr;
        return imageRefBuilder;
    }
}

void ImageFunctionHLSL::OutputImageFunctionArgumentList(
    TInfoSinkBase &out,
    const ImageFunctionHLSL::ImageFunction &imageFunction)
{
    out << "uint imageIndex";

    if (imageFunction.method == ImageFunctionHLSL::ImageFunction::Method::LOAD ||
        imageFunction.method == ImageFunctionHLSL::ImageFunction::Method::STORE)
    {
        switch (imageFunction.image)
        {
            case EbtImage2D:
            case EbtIImage2D:
            case EbtUImage2D:
                out << ", int2 p";
                break;
            case EbtImage3D:
            case EbtIImage3D:
            case EbtUImage3D:
            case EbtImageCube:
            case EbtIImageCube:
            case EbtUImageCube:
            case EbtImage2DArray:
            case EbtIImage2DArray:
            case EbtUImage2DArray:
                out << ", int3 p";
                break;
            case EbtUImageBuffer:
            case EbtIImageBuffer:
            case EbtImageBuffer:
                out << ", int p";
                break;

            default:
                UNREACHABLE();
        }

        if (imageFunction.method == ImageFunctionHLSL::ImageFunction::Method::STORE)
        {
            switch (imageFunction.image)
            {
                case EbtImage2D:
                case EbtImage3D:
                case EbtImageCube:
                case EbtImage2DArray:
                case EbtImageBuffer:
                    out << ", float4 data";
                    break;
                case EbtIImage2D:
                case EbtIImage3D:
                case EbtIImageCube:
                case EbtIImage2DArray:
                case EbtIImageBuffer:
                    out << ", int4 data";
                    break;
                case EbtUImage2D:
                case EbtUImage3D:
                case EbtUImageCube:
                case EbtUImage2DArray:
                case EbtUImageBuffer:
                    out << ", uint4 data";
                    break;
                default:
                    UNREACHABLE();
            }
        }
    }
}

// static
void ImageFunctionHLSL::OutputImageSizeFunctionBody(
    TInfoSinkBase &out,
    const ImageFunctionHLSL::ImageFunction &imageFunction,
    const ImmutableString &imageReference)
{
    if (IsImage3D(imageFunction.image) || IsImage2DArray(imageFunction.image) ||
        IsImageCube(imageFunction.image))
    {
        // "depth" stores either the number of layers in an array texture or 3D depth
        out << "    uint width; uint height; uint depth;\n"
            << "    " << imageReference << ".GetDimensions(width, height, depth);\n";
    }
    else if (IsImage2D(imageFunction.image))
    {
        out << "    uint width; uint height;\n"
            << "    " << imageReference << ".GetDimensions(width, height);\n";
    }
    else if (IsImageBuffer(imageFunction.image))
    {
        out << "    uint width;\n"
            << "    " << imageReference << ".GetDimensions(width);\n";
    }
    else
        UNREACHABLE();

    if (strcmp(imageFunction.getReturnType(), "int3") == 0)
    {
        out << "    return int3(width, height, depth);\n";
    }
    else if (strcmp(imageFunction.getReturnType(), "int2") == 0)
    {
        out << "    return int2(width, height);\n";
    }
    else if (strcmp(imageFunction.getReturnType(), "int") == 0)
        out << "    return int(width);\n";
    else
        UNREACHABLE();
}

// static
void ImageFunctionHLSL::OutputImageLoadFunctionBody(
    TInfoSinkBase &out,
    const ImageFunctionHLSL::ImageFunction &imageFunction,
    const ImmutableString &imageReference)
{
    if (IsImage3D(imageFunction.image) || IsImage2DArray(imageFunction.image) ||
        IsImageCube(imageFunction.image))
    {
        out << "    return " << imageReference << "[uint3(p.x, p.y, p.z)];\n";
    }
    else if (IsImage2D(imageFunction.image))
    {
        out << "    return " << imageReference << "[uint2(p.x, p.y)];\n";
    }
    else if (IsImageBuffer(imageFunction.image))
    {
        out << "    return " << imageReference << "[uint(p.x)];\n";
    }
    else
        UNREACHABLE();
}

// static
void ImageFunctionHLSL::OutputImageStoreFunctionBody(
    TInfoSinkBase &out,
    const ImageFunctionHLSL::ImageFunction &imageFunction,
    const ImmutableString &imageReference)
{
    if (IsImage3D(imageFunction.image) || IsImage2DArray(imageFunction.image) ||
        IsImage2D(imageFunction.image) || IsImageCube(imageFunction.image) ||
        IsImageBuffer(imageFunction.image))
    {
        out << "    " << imageReference << "[p] = data;\n";
    }
    else
        UNREACHABLE();
}

ImmutableString ImageFunctionHLSL::ImageFunction::name() const
{
    static const ImmutableString kGlImageName("gl_image");

    ImmutableString suffix(nullptr);
    if (readonly)
    {
        suffix = ImmutableString(TextureTypeSuffix(image, imageInternalFormat));
    }
    else
    {
        suffix = ImmutableString(RWTextureTypeSuffix(image, imageInternalFormat));
    }

    ImmutableStringBuilder name(kGlImageName.length() + suffix.length() + 5u);

    name << kGlImageName << suffix;

    switch (method)
    {
        case Method::SIZE:
            name << "Size";
            break;
        case Method::LOAD:
            name << "Load";
            break;
        case Method::STORE:
            name << "Store";
            break;
        default:
            UNREACHABLE();
    }

    return name;
}

ImageFunctionHLSL::ImageFunction::DataType ImageFunctionHLSL::ImageFunction::getDataType(
    TLayoutImageInternalFormat format) const
{
    switch (format)
    {
        case EiifRGBA32F:
        case EiifRGBA16F:
        case EiifR32F:
            return ImageFunction::DataType::FLOAT4;
        case EiifRGBA32UI:
        case EiifRGBA16UI:
        case EiifRGBA8UI:
        case EiifR32UI:
            return ImageFunction::DataType::UINT4;
        case EiifRGBA32I:
        case EiifRGBA16I:
        case EiifRGBA8I:
        case EiifR32I:
            return ImageFunction::DataType::INT4;
        case EiifRGBA8:
            return ImageFunction::DataType::UNORM_FLOAT4;
        case EiifRGBA8_SNORM:
            return ImageFunction::DataType::SNORM_FLOAT4;
        default:
            UNREACHABLE();
    }

    return ImageFunction::DataType::NONE;
}

const char *ImageFunctionHLSL::ImageFunction::getReturnType() const
{
    if (method == ImageFunction::Method::SIZE)
    {
        switch (image)
        {
            case EbtImage2D:
            case EbtIImage2D:
            case EbtUImage2D:
            case EbtImageCube:
            case EbtIImageCube:
            case EbtUImageCube:
                return "int2";
            case EbtImage3D:
            case EbtIImage3D:
            case EbtUImage3D:
            case EbtImage2DArray:
            case EbtIImage2DArray:
            case EbtUImage2DArray:
                return "int3";
            case EbtImageBuffer:
            case EbtIImageBuffer:
            case EbtUImageBuffer:
                return "int";
            default:
                UNREACHABLE();
        }
    }
    else if (method == ImageFunction::Method::LOAD)
    {
        switch (image)
        {
            case EbtImageBuffer:
            case EbtImage2D:
            case EbtImage3D:
            case EbtImageCube:
            case EbtImage2DArray:
                return "float4";
            case EbtIImageBuffer:
            case EbtIImage2D:
            case EbtIImage3D:
            case EbtIImageCube:
            case EbtIImage2DArray:
                return "int4";
            case EbtUImageBuffer:
            case EbtUImage2D:
            case EbtUImage3D:
            case EbtUImageCube:
            case EbtUImage2DArray:
                return "uint4";
            default:
                UNREACHABLE();
        }
    }
    else if (method == ImageFunction::Method::STORE)
    {
        return "void";
    }
    else
    {
        UNREACHABLE();
    }
    return "";
}

bool ImageFunctionHLSL::ImageFunction::operator<(const ImageFunction &rhs) const
{
    return std::tie(image, type, method, readonly) <
           std::tie(rhs.image, rhs.type, rhs.method, rhs.readonly);
}

ImmutableString ImageFunctionHLSL::useImageFunction(const ImmutableString &name,
                                                    const TBasicType &type,
                                                    TLayoutImageInternalFormat imageInternalFormat,
                                                    bool readonly)
{
    ASSERT(IsImage(type));
    ImageFunction imageFunction;
    imageFunction.image               = type;
    imageFunction.imageInternalFormat = imageInternalFormat;
    imageFunction.readonly            = readonly;
    imageFunction.type                = imageFunction.getDataType(imageInternalFormat);

    if (name == "imageSize")
    {
        imageFunction.method = ImageFunction::Method::SIZE;
    }
    else if (name == "imageLoad")
    {
        imageFunction.method = ImageFunction::Method::LOAD;
    }
    else if (name == "imageStore")
    {
        imageFunction.method = ImageFunction::Method::STORE;
    }
    else
        UNREACHABLE();

    mUsesImage.insert(imageFunction);
    return imageFunction.name();
}

void ImageFunctionHLSL::imageFunctionHeader(TInfoSinkBase &out)
{
    for (const ImageFunction &imageFunction : mUsesImage)
    {
        // Skip to generate image2D functions here, dynamically generate these
        // functions when linking, or after dispatch or draw.
        if (IsImage2D(imageFunction.image))
        {
            mUsedImage2DFunctionNames.insert(imageFunction.name().data());
            continue;
        }
        // Function header
        out << imageFunction.getReturnType() << " " << imageFunction.name() << "(";

        OutputImageFunctionArgumentList(out, imageFunction);

        out << ")\n"
               "{\n";

        ImmutableString imageReference = GetImageReference(out, imageFunction);
        if (imageFunction.method == ImageFunction::Method::SIZE)
        {
            OutputImageSizeFunctionBody(out, imageFunction, imageReference);
        }
        else if (imageFunction.method == ImageFunction::Method::LOAD)
        {
            OutputImageLoadFunctionBody(out, imageFunction, imageReference);
        }
        else
        {
            OutputImageStoreFunctionBody(out, imageFunction, imageReference);
        }

        out << "}\n"
               "\n";
    }
}

}  // namespace sh
