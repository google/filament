// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/common/IOSurfaceUtils.h"

#include <CoreFoundation/CoreFoundation.h>
#include <CoreVideo/CVPixelBuffer.h>

#include "dawn/common/Assert.h"
#include "dawn/common/CoreFoundationRef.h"

namespace dawn {

namespace {

void AddIntegerValue(CFMutableDictionaryRef dictionary, const CFStringRef key, int32_t value) {
    CFRef<CFNumberRef> number = AcquireCFRef(CFNumberCreate(nullptr, kCFNumberSInt32Type, &value));
    CFDictionaryAddValue(dictionary, key, number.Get());
}

OSType ToCVFormat(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
            return kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
        case wgpu::TextureFormat::R8BG8Biplanar422Unorm:
            return kCVPixelFormatType_422YpCbCr8BiPlanarVideoRange;
        case wgpu::TextureFormat::R8BG8Biplanar444Unorm:
            return kCVPixelFormatType_444YpCbCr8BiPlanarVideoRange;
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
            return kCVPixelFormatType_420YpCbCr10BiPlanarVideoRange;
        case wgpu::TextureFormat::R10X6BG10X6Biplanar422Unorm:
            return kCVPixelFormatType_422YpCbCr10BiPlanarVideoRange;
        case wgpu::TextureFormat::R10X6BG10X6Biplanar444Unorm:
            return kCVPixelFormatType_444YpCbCr10BiPlanarVideoRange;
        case wgpu::TextureFormat::R8BG8A8Triplanar420Unorm:
            return kCVPixelFormatType_420YpCbCr8VideoRange_8A_TriPlanar;
        default:
            DAWN_UNREACHABLE();
            return 0;
    }
}

uint32_t NumPlanes(wgpu::TextureFormat format) {
    switch (format) {
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
        case wgpu::TextureFormat::R8BG8Biplanar422Unorm:
        case wgpu::TextureFormat::R8BG8Biplanar444Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar422Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar444Unorm:
            return 2;
        case wgpu::TextureFormat::R8BG8A8Triplanar420Unorm:
            return 3;
        default:
            DAWN_UNREACHABLE();
            return 1;
    }
}

size_t GetSubSamplingFactorPerPlane(wgpu::TextureFormat format, size_t plane, bool isHorizontal) {
    switch (format) {
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
        case wgpu::TextureFormat::R8BG8A8Triplanar420Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
            return plane == 1 ? 2 : 1;
        case wgpu::TextureFormat::R8BG8Biplanar422Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar422Unorm:
            return plane == 1 && isHorizontal ? 2 : 1;
        case wgpu::TextureFormat::R8BG8Biplanar444Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar444Unorm:
            return 1;
        default:
            DAWN_UNREACHABLE();
            return 0;
    }
}

size_t BytesPerElement(wgpu::TextureFormat format, size_t plane) {
    switch (format) {
        case wgpu::TextureFormat::R8BG8Biplanar420Unorm:
        case wgpu::TextureFormat::R8BG8Biplanar422Unorm:
        case wgpu::TextureFormat::R8BG8Biplanar444Unorm:
        case wgpu::TextureFormat::R8BG8A8Triplanar420Unorm:
            return plane == 1 ? 2 : 1;
        case wgpu::TextureFormat::R10X6BG10X6Biplanar420Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar422Unorm:
        case wgpu::TextureFormat::R10X6BG10X6Biplanar444Unorm:
            return plane == 1 ? 4 : 2;
        default:
            DAWN_UNREACHABLE();
            return 0;
    }
}

}  // namespace

IOSurfaceRef CreateMultiPlanarIOSurface(wgpu::TextureFormat format,
                                        uint32_t width,
                                        uint32_t height) {
    CFRef<CFMutableDictionaryRef> dict = AcquireCFRef(CFDictionaryCreateMutable(
        kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks));
    AddIntegerValue(dict.Get(), kIOSurfaceWidth, width);
    AddIntegerValue(dict.Get(), kIOSurfaceHeight, height);
    AddIntegerValue(dict.Get(), kIOSurfacePixelFormat, ToCVFormat(format));

    size_t numPlanes = NumPlanes(format);

    CFRef<CFMutableArrayRef> planes =
        AcquireCFRef(CFArrayCreateMutable(kCFAllocatorDefault, numPlanes, &kCFTypeArrayCallBacks));
    size_t totalBytesAlloc = 0;
    for (size_t plane = 0; plane < numPlanes; ++plane) {
        const size_t planeWidth =
            width / GetSubSamplingFactorPerPlane(format, plane, /*isHorizontal=*/true);
        const size_t planeHeight =
            height / GetSubSamplingFactorPerPlane(format, plane, /*isHorizontal=*/false);
        const size_t planeBytesPerElement = BytesPerElement(format, plane);
        const size_t planeBytesPerRow =
            IOSurfaceAlignProperty(kIOSurfacePlaneBytesPerRow, planeWidth * planeBytesPerElement);
        const size_t planeBytesAlloc =
            IOSurfaceAlignProperty(kIOSurfacePlaneSize, planeHeight * planeBytesPerRow);
        const size_t planeOffset = IOSurfaceAlignProperty(kIOSurfacePlaneOffset, totalBytesAlloc);

        CFRef<CFMutableDictionaryRef> planeInfo = AcquireCFRef(
            CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks));

        AddIntegerValue(planeInfo.Get(), kIOSurfacePlaneWidth, planeWidth);
        AddIntegerValue(planeInfo.Get(), kIOSurfacePlaneHeight, planeHeight);
        AddIntegerValue(planeInfo.Get(), kIOSurfacePlaneBytesPerElement, planeBytesPerElement);
        AddIntegerValue(planeInfo.Get(), kIOSurfacePlaneBytesPerRow, planeBytesPerRow);
        AddIntegerValue(planeInfo.Get(), kIOSurfacePlaneSize, planeBytesAlloc);
        AddIntegerValue(planeInfo.Get(), kIOSurfacePlaneOffset, planeOffset);
        CFArrayAppendValue(planes.Get(), planeInfo.Get());
        totalBytesAlloc = planeOffset + planeBytesAlloc;
    }
    CFDictionaryAddValue(dict.Get(), kIOSurfacePlaneInfo, planes.Get());

    totalBytesAlloc = IOSurfaceAlignProperty(kIOSurfaceAllocSize, totalBytesAlloc);
    AddIntegerValue(dict.Get(), kIOSurfaceAllocSize, totalBytesAlloc);

    IOSurfaceRef surface = IOSurfaceCreate(dict.Get());

    return surface;
}
}  // namespace dawn
