//
// Copyright 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// loadimage.h: Defines image loading functions

#ifndef IMAGEUTIL_LOADIMAGE_H_
#define IMAGEUTIL_LOADIMAGE_H_

#include <stddef.h>
#include <stdint.h>
#include <memory>

namespace angle
{
class WorkerThreadPool;
struct ImageLoadContext
{
    // Satisfy chromium-style
    ImageLoadContext();
    ~ImageLoadContext();
    ImageLoadContext(const ImageLoadContext &other);

    // Passed to Load* functions as the context
    std::shared_ptr<WorkerThreadPool> singleThreadPool;
    std::shared_ptr<WorkerThreadPool> multiThreadPool;
};

void LoadA8ToRGBA8(const ImageLoadContext &context,
                   size_t width,
                   size_t height,
                   size_t depth,
                   const uint8_t *input,
                   size_t inputRowPitch,
                   size_t inputDepthPitch,
                   uint8_t *output,
                   size_t outputRowPitch,
                   size_t outputDepthPitch);

void LoadA8ToBGRA8(const ImageLoadContext &context,
                   size_t width,
                   size_t height,
                   size_t depth,
                   const uint8_t *input,
                   size_t inputRowPitch,
                   size_t inputDepthPitch,
                   uint8_t *output,
                   size_t outputRowPitch,
                   size_t outputDepthPitch);

void LoadA32FToRGBA32F(const ImageLoadContext &context,
                       size_t width,
                       size_t height,
                       size_t depth,
                       const uint8_t *input,
                       size_t inputRowPitch,
                       size_t inputDepthPitch,
                       uint8_t *output,
                       size_t outputRowPitch,
                       size_t outputDepthPitch);

void LoadA16FToRGBA16F(const ImageLoadContext &context,
                       size_t width,
                       size_t height,
                       size_t depth,
                       const uint8_t *input,
                       size_t inputRowPitch,
                       size_t inputDepthPitch,
                       uint8_t *output,
                       size_t outputRowPitch,
                       size_t outputDepthPitch);

void LoadL8ToRGBA8(const ImageLoadContext &context,
                   size_t width,
                   size_t height,
                   size_t depth,
                   const uint8_t *input,
                   size_t inputRowPitch,
                   size_t inputDepthPitch,
                   uint8_t *output,
                   size_t outputRowPitch,
                   size_t outputDepthPitch);

void LoadL8ToBGRA8(const ImageLoadContext &context,
                   size_t width,
                   size_t height,
                   size_t depth,
                   const uint8_t *input,
                   size_t inputRowPitch,
                   size_t inputDepthPitch,
                   uint8_t *output,
                   size_t outputRowPitch,
                   size_t outputDepthPitch);

void LoadL32FToRGBA32F(const ImageLoadContext &context,
                       size_t width,
                       size_t height,
                       size_t depth,
                       const uint8_t *input,
                       size_t inputRowPitch,
                       size_t inputDepthPitch,
                       uint8_t *output,
                       size_t outputRowPitch,
                       size_t outputDepthPitch);

void LoadL16FToRGBA16F(const ImageLoadContext &context,
                       size_t width,
                       size_t height,
                       size_t depth,
                       const uint8_t *input,
                       size_t inputRowPitch,
                       size_t inputDepthPitch,
                       uint8_t *output,
                       size_t outputRowPitch,
                       size_t outputDepthPitch);

void LoadLA8ToRGBA4(const ImageLoadContext &context,
                    size_t width,
                    size_t height,
                    size_t depth,
                    const uint8_t *input,
                    size_t inputRowPitch,
                    size_t inputDepthPitch,
                    uint8_t *output,
                    size_t outputRowPitch,
                    size_t outputDepthPitch);

void LoadLA8ToRGBA8(const ImageLoadContext &context,
                    size_t width,
                    size_t height,
                    size_t depth,
                    const uint8_t *input,
                    size_t inputRowPitch,
                    size_t inputDepthPitch,
                    uint8_t *output,
                    size_t outputRowPitch,
                    size_t outputDepthPitch);

void LoadLA8ToBGRA8(const ImageLoadContext &context,
                    size_t width,
                    size_t height,
                    size_t depth,
                    const uint8_t *input,
                    size_t inputRowPitch,
                    size_t inputDepthPitch,
                    uint8_t *output,
                    size_t outputRowPitch,
                    size_t outputDepthPitch);

void LoadLA32FToRGBA32F(const ImageLoadContext &context,
                        size_t width,
                        size_t height,
                        size_t depth,
                        const uint8_t *input,
                        size_t inputRowPitch,
                        size_t inputDepthPitch,
                        uint8_t *output,
                        size_t outputRowPitch,
                        size_t outputDepthPitch);

void LoadLA16FToRGBA16F(const ImageLoadContext &context,
                        size_t width,
                        size_t height,
                        size_t depth,
                        const uint8_t *input,
                        size_t inputRowPitch,
                        size_t inputDepthPitch,
                        uint8_t *output,
                        size_t outputRowPitch,
                        size_t outputDepthPitch);

void LoadRGB8ToBGR565(const ImageLoadContext &context,
                      size_t width,
                      size_t height,
                      size_t depth,
                      const uint8_t *input,
                      size_t inputRowPitch,
                      size_t inputDepthPitch,
                      uint8_t *output,
                      size_t outputRowPitch,
                      size_t outputDepthPitch);

void LoadRGB565ToBGR565(const ImageLoadContext &context,
                        size_t width,
                        size_t height,
                        size_t depth,
                        const uint8_t *input,
                        size_t inputRowPitch,
                        size_t inputDepthPitch,
                        uint8_t *output,
                        size_t outputRowPitch,
                        size_t outputDepthPitch);

void LoadRGB8ToBGRX8(const ImageLoadContext &context,
                     size_t width,
                     size_t height,
                     size_t depth,
                     const uint8_t *input,
                     size_t inputRowPitch,
                     size_t inputDepthPitch,
                     uint8_t *output,
                     size_t outputRowPitch,
                     size_t outputDepthPitch);

void LoadRG8ToBGRX8(const ImageLoadContext &context,
                    size_t width,
                    size_t height,
                    size_t depth,
                    const uint8_t *input,
                    size_t inputRowPitch,
                    size_t inputDepthPitch,
                    uint8_t *output,
                    size_t outputRowPitch,
                    size_t outputDepthPitch);

void LoadR8ToBGRX8(const ImageLoadContext &context,
                   size_t width,
                   size_t height,
                   size_t depth,
                   const uint8_t *input,
                   size_t inputRowPitch,
                   size_t inputDepthPitch,
                   uint8_t *output,
                   size_t outputRowPitch,
                   size_t outputDepthPitch);

void LoadR5G6B5ToBGRA8(const ImageLoadContext &context,
                       size_t width,
                       size_t height,
                       size_t depth,
                       const uint8_t *input,
                       size_t inputRowPitch,
                       size_t inputDepthPitch,
                       uint8_t *output,
                       size_t outputRowPitch,
                       size_t outputDepthPitch);

void LoadR5G6B5ToRGBA8(const ImageLoadContext &context,
                       size_t width,
                       size_t height,
                       size_t depth,
                       const uint8_t *input,
                       size_t inputRowPitch,
                       size_t inputDepthPitch,
                       uint8_t *output,
                       size_t outputRowPitch,
                       size_t outputDepthPitch);

void LoadRGBA8ToBGRA8(const ImageLoadContext &context,
                      size_t width,
                      size_t height,
                      size_t depth,
                      const uint8_t *input,
                      size_t inputRowPitch,
                      size_t inputDepthPitch,
                      uint8_t *output,
                      size_t outputRowPitch,
                      size_t outputDepthPitch);

void LoadRGBA8ToBGRA4(const ImageLoadContext &context,
                      size_t width,
                      size_t height,
                      size_t depth,
                      const uint8_t *input,
                      size_t inputRowPitch,
                      size_t inputDepthPitch,
                      uint8_t *output,
                      size_t outputRowPitch,
                      size_t outputDepthPitch);

void LoadRGBA8ToRGBA4(const ImageLoadContext &context,
                      size_t width,
                      size_t height,
                      size_t depth,
                      const uint8_t *input,
                      size_t inputRowPitch,
                      size_t inputDepthPitch,
                      uint8_t *output,
                      size_t outputRowPitch,
                      size_t outputDepthPitch);

void LoadRGBA4ToARGB4(const ImageLoadContext &context,
                      size_t width,
                      size_t height,
                      size_t depth,
                      const uint8_t *input,
                      size_t inputRowPitch,
                      size_t inputDepthPitch,
                      uint8_t *output,
                      size_t outputRowPitch,
                      size_t outputDepthPitch);

void LoadRGBA4ToRGBA4(const ImageLoadContext &context,
                      size_t width,
                      size_t height,
                      size_t depth,
                      const uint8_t *input,
                      size_t inputRowPitch,
                      size_t inputDepthPitch,
                      uint8_t *output,
                      size_t outputRowPitch,
                      size_t outputDepthPitch);

void LoadRGBA4ToBGRA8(const ImageLoadContext &context,
                      size_t width,
                      size_t height,
                      size_t depth,
                      const uint8_t *input,
                      size_t inputRowPitch,
                      size_t inputDepthPitch,
                      uint8_t *output,
                      size_t outputRowPitch,
                      size_t outputDepthPitch);

void LoadRGBA4ToRGBA8(const ImageLoadContext &context,
                      size_t width,
                      size_t height,
                      size_t depth,
                      const uint8_t *input,
                      size_t inputRowPitch,
                      size_t inputDepthPitch,
                      uint8_t *output,
                      size_t outputRowPitch,
                      size_t outputDepthPitch);

void LoadBGRA4ToBGRA8(const ImageLoadContext &context,
                      size_t width,
                      size_t height,
                      size_t depth,
                      const uint8_t *input,
                      size_t inputRowPitch,
                      size_t inputDepthPitch,
                      uint8_t *output,
                      size_t outputRowPitch,
                      size_t outputDepthPitch);

void LoadRGBA8ToBGR5A1(const ImageLoadContext &context,
                       size_t width,
                       size_t height,
                       size_t depth,
                       const uint8_t *input,
                       size_t inputRowPitch,
                       size_t inputDepthPitch,
                       uint8_t *output,
                       size_t outputRowPitch,
                       size_t outputDepthPitch);

void LoadRGBA8ToRGB5A1(const ImageLoadContext &context,
                       size_t width,
                       size_t height,
                       size_t depth,
                       const uint8_t *input,
                       size_t inputRowPitch,
                       size_t inputDepthPitch,
                       uint8_t *output,
                       size_t outputRowPitch,
                       size_t outputDepthPitch);

void LoadRGB10A2ToBGR5A1(const ImageLoadContext &context,
                         size_t width,
                         size_t height,
                         size_t depth,
                         const uint8_t *input,
                         size_t inputRowPitch,
                         size_t inputDepthPitch,
                         uint8_t *output,
                         size_t outputRowPitch,
                         size_t outputDepthPitch);

void LoadRGB10A2ToRGB5A1(const ImageLoadContext &context,
                         size_t width,
                         size_t height,
                         size_t depth,
                         const uint8_t *input,
                         size_t inputRowPitch,
                         size_t inputDepthPitch,
                         uint8_t *output,
                         size_t outputRowPitch,
                         size_t outputDepthPitch);

void LoadRGB10A2ToRGB565(const ImageLoadContext &context,
                         size_t width,
                         size_t height,
                         size_t depth,
                         const uint8_t *input,
                         size_t inputRowPitch,
                         size_t inputDepthPitch,
                         uint8_t *output,
                         size_t outputRowPitch,
                         size_t outputDepthPitch);

void LoadRGB5A1ToRGB5A1(const ImageLoadContext &context,
                        size_t width,
                        size_t height,
                        size_t depth,
                        const uint8_t *input,
                        size_t inputRowPitch,
                        size_t inputDepthPitch,
                        uint8_t *output,
                        size_t outputRowPitch,
                        size_t outputDepthPitch);

void LoadRGB5A1ToBGR5A1(const ImageLoadContext &context,
                        size_t width,
                        size_t height,
                        size_t depth,
                        const uint8_t *input,
                        size_t inputRowPitch,
                        size_t inputDepthPitch,
                        uint8_t *output,
                        size_t outputRowPitch,
                        size_t outputDepthPitch);

void LoadRGB5A1ToA1RGB5(const ImageLoadContext &context,
                        size_t width,
                        size_t height,
                        size_t depth,
                        const uint8_t *input,
                        size_t inputRowPitch,
                        size_t inputDepthPitch,
                        uint8_t *output,
                        size_t outputRowPitch,
                        size_t outputDepthPitch);

void LoadRGB5A1ToBGRA8(const ImageLoadContext &context,
                       size_t width,
                       size_t height,
                       size_t depth,
                       const uint8_t *input,
                       size_t inputRowPitch,
                       size_t inputDepthPitch,
                       uint8_t *output,
                       size_t outputRowPitch,
                       size_t outputDepthPitch);

void LoadRGB5A1ToRGBA8(const ImageLoadContext &context,
                       size_t width,
                       size_t height,
                       size_t depth,
                       const uint8_t *input,
                       size_t inputRowPitch,
                       size_t inputDepthPitch,
                       uint8_t *output,
                       size_t outputRowPitch,
                       size_t outputDepthPitch);

void LoadBGR5A1ToBGRA8(const ImageLoadContext &context,
                       size_t width,
                       size_t height,
                       size_t depth,
                       const uint8_t *input,
                       size_t inputRowPitch,
                       size_t inputDepthPitch,
                       uint8_t *output,
                       size_t outputRowPitch,
                       size_t outputDepthPitch);
void LoadRGB10A2ToRGB8(const ImageLoadContext &context,
                       size_t width,
                       size_t height,
                       size_t depth,
                       const uint8_t *input,
                       size_t inputRowPitch,
                       size_t inputDepthPitch,
                       uint8_t *output,
                       size_t outputRowPitch,
                       size_t outputDepthPitch);

void LoadRGB10A2ToRGBA8(const ImageLoadContext &context,
                        size_t width,
                        size_t height,
                        size_t depth,
                        const uint8_t *input,
                        size_t inputRowPitch,
                        size_t inputDepthPitch,
                        uint8_t *output,
                        size_t outputRowPitch,
                        size_t outputDepthPitch);

void LoadRGB10A2ToRGB10X2(const ImageLoadContext &context,
                          size_t width,
                          size_t height,
                          size_t depth,
                          const uint8_t *input,
                          size_t inputRowPitch,
                          size_t inputDepthPitch,
                          uint8_t *output,
                          size_t outputRowPitch,
                          size_t outputDepthPitch);

void LoadBGR10A2ToRGB10A2(const ImageLoadContext &context,
                          size_t width,
                          size_t height,
                          size_t depth,
                          const uint8_t *input,
                          size_t inputRowPitch,
                          size_t inputDepthPitch,
                          uint8_t *output,
                          size_t outputRowPitch,
                          size_t outputDepthPitch);

void LoadRGB16FToRGB9E5(const ImageLoadContext &context,
                        size_t width,
                        size_t height,
                        size_t depth,
                        const uint8_t *input,
                        size_t inputRowPitch,
                        size_t inputDepthPitch,
                        uint8_t *output,
                        size_t outputRowPitch,
                        size_t outputDepthPitch);

void LoadRGB32FToRGB9E5(const ImageLoadContext &context,
                        size_t width,
                        size_t height,
                        size_t depth,
                        const uint8_t *input,
                        size_t inputRowPitch,
                        size_t inputDepthPitch,
                        uint8_t *output,
                        size_t outputRowPitch,
                        size_t outputDepthPitch);

void LoadRGB16FToRG11B10F(const ImageLoadContext &context,
                          size_t width,
                          size_t height,
                          size_t depth,
                          const uint8_t *input,
                          size_t inputRowPitch,
                          size_t inputDepthPitch,
                          uint8_t *output,
                          size_t outputRowPitch,
                          size_t outputDepthPitch);

void LoadRGB32FToRG11B10F(const ImageLoadContext &context,
                          size_t width,
                          size_t height,
                          size_t depth,
                          const uint8_t *input,
                          size_t inputRowPitch,
                          size_t inputDepthPitch,
                          uint8_t *output,
                          size_t outputRowPitch,
                          size_t outputDepthPitch);

void LoadD24S8ToS8D24(const ImageLoadContext &context,
                      size_t width,
                      size_t height,
                      size_t depth,
                      const uint8_t *input,
                      size_t inputRowPitch,
                      size_t inputDepthPitch,
                      uint8_t *output,
                      size_t outputRowPitch,
                      size_t outputDepthPitch);

void LoadD24S8ToD32FS8X24(const ImageLoadContext &context,
                          size_t width,
                          size_t height,
                          size_t depth,
                          const uint8_t *input,
                          size_t inputRowPitch,
                          size_t inputDepthPitch,
                          uint8_t *output,
                          size_t outputRowPitch,
                          size_t outputDepthPitch);

void LoadD24S8ToD32F(const ImageLoadContext &context,
                     size_t width,
                     size_t height,
                     size_t depth,
                     const uint8_t *input,
                     size_t inputRowPitch,
                     size_t inputDepthPitch,
                     uint8_t *output,
                     size_t outputRowPitch,
                     size_t outputDepthPitch);

void LoadD32ToD32FX32(const ImageLoadContext &context,
                      size_t width,
                      size_t height,
                      size_t depth,
                      const uint8_t *input,
                      size_t inputRowPitch,
                      size_t inputDepthPitch,
                      uint8_t *output,
                      size_t outputRowPitch,
                      size_t outputDepthPitch);

void LoadD32ToD32F(const ImageLoadContext &context,
                   size_t width,
                   size_t height,
                   size_t depth,
                   const uint8_t *input,
                   size_t inputRowPitch,
                   size_t inputDepthPitch,
                   uint8_t *output,
                   size_t outputRowPitch,
                   size_t outputDepthPitch);

void LoadD32FToD32F(const ImageLoadContext &context,
                    size_t width,
                    size_t height,
                    size_t depth,
                    const uint8_t *input,
                    size_t inputRowPitch,
                    size_t inputDepthPitch,
                    uint8_t *output,
                    size_t outputRowPitch,
                    size_t outputDepthPitch);

void LoadD32FS8X24ToS8D24(const ImageLoadContext &context,
                          size_t width,
                          size_t height,
                          size_t depth,
                          const uint8_t *input,
                          size_t inputRowPitch,
                          size_t inputDepthPitch,
                          uint8_t *output,
                          size_t outputRowPitch,
                          size_t outputDepthPitch);

void LoadX24S8ToS8(const ImageLoadContext &context,
                   size_t width,
                   size_t height,
                   size_t depth,
                   const uint8_t *input,
                   size_t inputRowPitch,
                   size_t inputDepthPitch,
                   uint8_t *output,
                   size_t outputRowPitch,
                   size_t outputDepthPitch);

void LoadX32S8ToS8(const ImageLoadContext &context,
                   size_t width,
                   size_t height,
                   size_t depth,
                   const uint8_t *input,
                   size_t inputRowPitch,
                   size_t inputDepthPitch,
                   uint8_t *output,
                   size_t outputRowPitch,
                   size_t outputDepthPitch);

void LoadD32FS8X24ToD32F(const ImageLoadContext &context,
                         size_t width,
                         size_t height,
                         size_t depth,
                         const uint8_t *input,
                         size_t inputRowPitch,
                         size_t inputDepthPitch,
                         uint8_t *output,
                         size_t outputRowPitch,
                         size_t outputDepthPitch);

void LoadD32FS8X24ToD32FS8X24(const ImageLoadContext &context,
                              size_t width,
                              size_t height,
                              size_t depth,
                              const uint8_t *input,
                              size_t inputRowPitch,
                              size_t inputDepthPitch,
                              uint8_t *output,
                              size_t outputRowPitch,
                              size_t outputDepthPitch);

template <typename type, size_t componentCount>
inline void LoadToNative(const ImageLoadContext &context,
                         size_t width,
                         size_t height,
                         size_t depth,
                         const uint8_t *input,
                         size_t inputRowPitch,
                         size_t inputDepthPitch,
                         uint8_t *output,
                         size_t outputRowPitch,
                         size_t outputDepthPitch);

template <typename type, uint32_t fourthComponentBits>
inline void LoadToNative3To4(const ImageLoadContext &context,
                             size_t width,
                             size_t height,
                             size_t depth,
                             const uint8_t *input,
                             size_t inputRowPitch,
                             size_t inputDepthPitch,
                             uint8_t *output,
                             size_t outputRowPitch,
                             size_t outputDepthPitch);

template <size_t componentCount>
inline void Load32FTo16F(const ImageLoadContext &context,
                         size_t width,
                         size_t height,
                         size_t depth,
                         const uint8_t *input,
                         size_t inputRowPitch,
                         size_t inputDepthPitch,
                         uint8_t *output,
                         size_t outputRowPitch,
                         size_t outputDepthPitch);

void LoadD16ToD32F(const ImageLoadContext &context,
                   size_t width,
                   size_t height,
                   size_t depth,
                   const uint8_t *input,
                   size_t inputRowPitch,
                   size_t inputDepthPitch,
                   uint8_t *output,
                   size_t outputRowPitch,
                   size_t outputDepthPitch);

void LoadS8ToS8X24(const ImageLoadContext &context,
                   size_t width,
                   size_t height,
                   size_t depth,
                   const uint8_t *input,
                   size_t inputRowPitch,
                   size_t inputDepthPitch,
                   uint8_t *output,
                   size_t outputRowPitch,
                   size_t outputDepthPitch);

void LoadRGB32FToRGBA16F(const ImageLoadContext &context,
                         size_t width,
                         size_t height,
                         size_t depth,
                         const uint8_t *input,
                         size_t inputRowPitch,
                         size_t inputDepthPitch,
                         uint8_t *output,
                         size_t outputRowPitch,
                         size_t outputDepthPitch);

void LoadRGB32FToRGB16F(const ImageLoadContext &context,
                        size_t width,
                        size_t height,
                        size_t depth,
                        const uint8_t *input,
                        size_t inputRowPitch,
                        size_t inputDepthPitch,
                        uint8_t *output,
                        size_t outputRowPitch,
                        size_t outputDepthPitch);

template <size_t blockWidth, size_t blockHeight, size_t blockDepth, size_t blockSize>
inline void LoadCompressedToNative(const ImageLoadContext &context,
                                   size_t width,
                                   size_t height,
                                   size_t depth,
                                   const uint8_t *input,
                                   size_t inputRowPitch,
                                   size_t inputDepthPitch,
                                   uint8_t *output,
                                   size_t outputRowPitch,
                                   size_t outputDepthPitch);

void LoadR32ToR16(const ImageLoadContext &context,
                  size_t width,
                  size_t height,
                  size_t depth,
                  const uint8_t *input,
                  size_t inputRowPitch,
                  size_t inputDepthPitch,
                  uint8_t *output,
                  size_t outputRowPitch,
                  size_t outputDepthPitch);

template <typename type,
          uint32_t firstBits,
          uint32_t secondBits,
          uint32_t thirdBits,
          uint32_t fourthBits>
inline void Initialize4ComponentData(const ImageLoadContext &context,
                                     size_t width,
                                     size_t height,
                                     size_t depth,
                                     uint8_t *output,
                                     size_t outputRowPitch,
                                     size_t outputDepthPitch);

void LoadD32ToX8D24(const ImageLoadContext &context,
                    size_t width,
                    size_t height,
                    size_t depth,
                    const uint8_t *input,
                    size_t inputRowPitch,
                    size_t inputDepthPitch,
                    uint8_t *output,
                    size_t outputRowPitch,
                    size_t outputDepthPitch);

void LoadETC1RGB8ToRGBA8(const ImageLoadContext &context,
                         size_t width,
                         size_t height,
                         size_t depth,
                         const uint8_t *input,
                         size_t inputRowPitch,
                         size_t inputDepthPitch,
                         uint8_t *output,
                         size_t outputRowPitch,
                         size_t outputDepthPitch);

void LoadASTCToRGBA8Inner(const ImageLoadContext &context,
                          size_t width,
                          size_t height,
                          size_t depth,
                          uint32_t blockWidth,
                          uint32_t blockHeight,
                          const uint8_t *input,
                          size_t inputRowPitch,
                          size_t inputDepthPitch,
                          uint8_t *output,
                          size_t outputRowPitch,
                          size_t outputDepthPitch);

template <size_t blockWidth, size_t blockHeight>
inline void LoadASTCToRGBA8(const ImageLoadContext &context,
                            size_t width,
                            size_t height,
                            size_t depth,
                            const uint8_t *input,
                            size_t inputRowPitch,
                            size_t inputDepthPitch,
                            uint8_t *output,
                            size_t outputRowPitch,
                            size_t outputDepthPitch);

void LoadETC1RGB8ToBC1(const ImageLoadContext &context,
                       size_t width,
                       size_t height,
                       size_t depth,
                       const uint8_t *input,
                       size_t inputRowPitch,
                       size_t inputDepthPitch,
                       uint8_t *output,
                       size_t outputRowPitch,
                       size_t outputDepthPitch);

void LoadEACR11ToR8(const ImageLoadContext &context,
                    size_t width,
                    size_t height,
                    size_t depth,
                    const uint8_t *input,
                    size_t inputRowPitch,
                    size_t inputDepthPitch,
                    uint8_t *output,
                    size_t outputRowPitch,
                    size_t outputDepthPitch);

void LoadEACR11SToR8(const ImageLoadContext &context,
                     size_t width,
                     size_t height,
                     size_t depth,
                     const uint8_t *input,
                     size_t inputRowPitch,
                     size_t inputDepthPitch,
                     uint8_t *output,
                     size_t outputRowPitch,
                     size_t outputDepthPitch);

void LoadEACRG11ToRG8(const ImageLoadContext &context,
                      size_t width,
                      size_t height,
                      size_t depth,
                      const uint8_t *input,
                      size_t inputRowPitch,
                      size_t inputDepthPitch,
                      uint8_t *output,
                      size_t outputRowPitch,
                      size_t outputDepthPitch);

void LoadEACRG11SToRG8(const ImageLoadContext &context,
                       size_t width,
                       size_t height,
                       size_t depth,
                       const uint8_t *input,
                       size_t inputRowPitch,
                       size_t inputDepthPitch,
                       uint8_t *output,
                       size_t outputRowPitch,
                       size_t outputDepthPitch);

void LoadEACR11ToR16(const ImageLoadContext &context,
                     size_t width,
                     size_t height,
                     size_t depth,
                     const uint8_t *input,
                     size_t inputRowPitch,
                     size_t inputDepthPitch,
                     uint8_t *output,
                     size_t outputRowPitch,
                     size_t outputDepthPitch);

void LoadEACR11SToR16(const ImageLoadContext &context,
                      size_t width,
                      size_t height,
                      size_t depth,
                      const uint8_t *input,
                      size_t inputRowPitch,
                      size_t inputDepthPitch,
                      uint8_t *output,
                      size_t outputRowPitch,
                      size_t outputDepthPitch);

void LoadEACRG11ToRG16(const ImageLoadContext &context,
                       size_t width,
                       size_t height,
                       size_t depth,
                       const uint8_t *input,
                       size_t inputRowPitch,
                       size_t inputDepthPitch,
                       uint8_t *output,
                       size_t outputRowPitch,
                       size_t outputDepthPitch);

void LoadEACRG11SToRG16(const ImageLoadContext &context,
                        size_t width,
                        size_t height,
                        size_t depth,
                        const uint8_t *input,
                        size_t inputRowPitch,
                        size_t inputDepthPitch,
                        uint8_t *output,
                        size_t outputRowPitch,
                        size_t outputDepthPitch);

void LoadEACR11ToR16F(const ImageLoadContext &context,
                      size_t width,
                      size_t height,
                      size_t depth,
                      const uint8_t *input,
                      size_t inputRowPitch,
                      size_t inputDepthPitch,
                      uint8_t *output,
                      size_t outputRowPitch,
                      size_t outputDepthPitch);

void LoadEACR11SToR16F(const ImageLoadContext &context,
                       size_t width,
                       size_t height,
                       size_t depth,
                       const uint8_t *input,
                       size_t inputRowPitch,
                       size_t inputDepthPitch,
                       uint8_t *output,
                       size_t outputRowPitch,
                       size_t outputDepthPitch);

void LoadEACRG11ToRG16F(const ImageLoadContext &context,
                        size_t width,
                        size_t height,
                        size_t depth,
                        const uint8_t *input,
                        size_t inputRowPitch,
                        size_t inputDepthPitch,
                        uint8_t *output,
                        size_t outputRowPitch,
                        size_t outputDepthPitch);

void LoadEACRG11SToRG16F(const ImageLoadContext &context,
                         size_t width,
                         size_t height,
                         size_t depth,
                         const uint8_t *input,
                         size_t inputRowPitch,
                         size_t inputDepthPitch,
                         uint8_t *output,
                         size_t outputRowPitch,
                         size_t outputDepthPitch);

void LoadETC2RGB8ToRGBA8(const ImageLoadContext &context,
                         size_t width,
                         size_t height,
                         size_t depth,
                         const uint8_t *input,
                         size_t inputRowPitch,
                         size_t inputDepthPitch,
                         uint8_t *output,
                         size_t outputRowPitch,
                         size_t outputDepthPitch);

void LoadETC2RGB8ToBC1(const ImageLoadContext &context,
                       size_t width,
                       size_t height,
                       size_t depth,
                       const uint8_t *input,
                       size_t inputRowPitch,
                       size_t inputDepthPitch,
                       uint8_t *output,
                       size_t outputRowPitch,
                       size_t outputDepthPitch);

void LoadETC2SRGB8ToRGBA8(const ImageLoadContext &context,
                          size_t width,
                          size_t height,
                          size_t depth,
                          const uint8_t *input,
                          size_t inputRowPitch,
                          size_t inputDepthPitch,
                          uint8_t *output,
                          size_t outputRowPitch,
                          size_t outputDepthPitch);

void LoadETC2SRGB8ToBC1(const ImageLoadContext &context,
                        size_t width,
                        size_t height,
                        size_t depth,
                        const uint8_t *input,
                        size_t inputRowPitch,
                        size_t inputDepthPitch,
                        uint8_t *output,
                        size_t outputRowPitch,
                        size_t outputDepthPitch);

void LoadETC2RGB8A1ToRGBA8(const ImageLoadContext &context,
                           size_t width,
                           size_t height,
                           size_t depth,
                           const uint8_t *input,
                           size_t inputRowPitch,
                           size_t inputDepthPitch,
                           uint8_t *output,
                           size_t outputRowPitch,
                           size_t outputDepthPitch);

void LoadETC2RGB8A1ToBC1(const ImageLoadContext &context,
                         size_t width,
                         size_t height,
                         size_t depth,
                         const uint8_t *input,
                         size_t inputRowPitch,
                         size_t inputDepthPitch,
                         uint8_t *output,
                         size_t outputRowPitch,
                         size_t outputDepthPitch);

void LoadETC2SRGB8A1ToRGBA8(const ImageLoadContext &context,
                            size_t width,
                            size_t height,
                            size_t depth,
                            const uint8_t *input,
                            size_t inputRowPitch,
                            size_t inputDepthPitch,
                            uint8_t *output,
                            size_t outputRowPitch,
                            size_t outputDepthPitch);

void LoadETC2SRGB8A1ToBC1(const ImageLoadContext &context,
                          size_t width,
                          size_t height,
                          size_t depth,
                          const uint8_t *input,
                          size_t inputRowPitch,
                          size_t inputDepthPitch,
                          uint8_t *output,
                          size_t outputRowPitch,
                          size_t outputDepthPitch);

void LoadETC2RGBA8ToBC3(const ImageLoadContext &context,
                        size_t width,
                        size_t height,
                        size_t depth,
                        const uint8_t *input,
                        size_t inputRowPitch,
                        size_t inputDepthPitch,
                        uint8_t *output,
                        size_t outputRowPitch,
                        size_t outputDepthPitch);

void LoadETC2SRGBA8ToBC3(const ImageLoadContext &context,
                         size_t width,
                         size_t height,
                         size_t depth,
                         const uint8_t *input,
                         size_t inputRowPitch,
                         size_t inputDepthPitch,
                         uint8_t *output,
                         size_t outputRowPitch,
                         size_t outputDepthPitch);

void LoadEACR11ToBC4(const ImageLoadContext &context,
                     size_t width,
                     size_t height,
                     size_t depth,
                     const uint8_t *input,
                     size_t inputRowPitch,
                     size_t inputDepthPitch,
                     uint8_t *output,
                     size_t outputRowPitch,
                     size_t outputDepthPitch);

void LoadEACR11SToBC4(const ImageLoadContext &context,
                      size_t width,
                      size_t height,
                      size_t depth,
                      const uint8_t *input,
                      size_t inputRowPitch,
                      size_t inputDepthPitch,
                      uint8_t *output,
                      size_t outputRowPitch,
                      size_t outputDepthPitch);

void LoadEACRG11ToBC5(const ImageLoadContext &context,
                      size_t width,
                      size_t height,
                      size_t depth,
                      const uint8_t *input,
                      size_t inputRowPitch,
                      size_t inputDepthPitch,
                      uint8_t *output,
                      size_t outputRowPitch,
                      size_t outputDepthPitch);

void LoadEACRG11SToBC5(const ImageLoadContext &context,
                       size_t width,
                       size_t height,
                       size_t depth,
                       const uint8_t *input,
                       size_t inputRowPitch,
                       size_t inputDepthPitch,
                       uint8_t *output,
                       size_t outputRowPitch,
                       size_t outputDepthPitch);

void LoadETC2RGBA8ToRGBA8(const ImageLoadContext &context,
                          size_t width,
                          size_t height,
                          size_t depth,
                          const uint8_t *input,
                          size_t inputRowPitch,
                          size_t inputDepthPitch,
                          uint8_t *output,
                          size_t outputRowPitch,
                          size_t outputDepthPitch);

void LoadETC2SRGBA8ToSRGBA8(const ImageLoadContext &context,
                            size_t width,
                            size_t height,
                            size_t depth,
                            const uint8_t *input,
                            size_t inputRowPitch,
                            size_t inputDepthPitch,
                            uint8_t *output,
                            size_t outputRowPitch,
                            size_t outputDepthPitch);

void LoadYuvToNative(const ImageLoadContext &context,
                     size_t width,
                     size_t height,
                     size_t depth,
                     const uint8_t *input,
                     size_t inputRowPitch,
                     size_t inputDepthPitch,
                     uint8_t *output,
                     size_t outputRowPitch,
                     size_t outputDepthPitch);

void LoadPalettedToRGBA8Impl(const ImageLoadContext &context,
                             size_t width,
                             size_t height,
                             size_t depth,
                             uint32_t indexBits,
                             uint32_t redBlueBits,
                             uint32_t greenBits,
                             uint32_t alphaBits,
                             const uint8_t *input,
                             size_t inputRowPitch,
                             size_t inputDepthPitch,
                             uint8_t *output,
                             size_t outputRowPitch,
                             size_t outputDepthPitch);

template <uint32_t indexBits, uint32_t redBlueBits, uint32_t greenBits, uint32_t alphaBits>
inline void LoadPalettedToRGBA8(const ImageLoadContext &context,
                                size_t width,
                                size_t height,
                                size_t depth,
                                const uint8_t *input,
                                size_t inputRowPitch,
                                size_t inputDepthPitch,
                                uint8_t *output,
                                size_t outputRowPitch,
                                size_t outputDepthPitch);

}  // namespace angle

#include "loadimage.inc"

#endif  // IMAGEUTIL_LOADIMAGE_H_
