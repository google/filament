/*
 * Copyright (C) 2026 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if FILAMENT_ENABLE_FGVIEWER

#include "fg/FgviewerManager.h"

#include "details/View.h"

#include "fg/FrameGraph.h"
#include "fg/FrameGraphId.h"
#include "fg/FrameGraphResources.h"
#include "fg/FrameGraphTexture.h"

#include <fgviewer/DebugServer.h>
#include <fgviewer/FrameGraphInfo.h>

#include <backend/DriverEnums.h>
#include <backend/Handle.h>
#include <backend/PixelBufferDescriptor.h>

#include <utils/CString.h>
#include <utils/compiler.h>
#include <utils/Log.h>

namespace filament {

FgviewerManager::FgviewerManager(FEngine& engine, utils::CString const& serverPort)
        : mServer(std::make_unique<fgviewer::DebugServer>(atoi(serverPort.c_str()),
                  [this](fgviewer::ViewHandle viewId, uint32_t id, utils::CString const& name,
                          Request::Callback&& callback) {
                      requestTextureReadback(viewId, id, name, std::move(callback));
                  })),
          mPostProcessManager(engine.getPostProcessManager()) {}

bool FgviewerManager::isReady() const { return mServer->isReady(); }

fgviewer::ViewHandle FgviewerManager::createView(utils::CString name) {
    return mServer->createView(std::move(name));
}

void FgviewerManager::destroyView(fgviewer::ViewHandle h) {
    mServer->destroyView(h);
}

void FgviewerManager::addReadbacksToFramegraph(FrameGraph& fg,
        backend::Handle<backend::HwRenderTarget> viewTarget, uint32_t viewId) {
    std::vector<Request> requests;

    {
        std::unique_lock<utils::Mutex> lock(mReadbackRequestsMutex);
        std::swap(requests, mReadbackRequests);
    }

    // Keep requests that belong to other views in the queue
    std::vector<Request> remainingRequests;

    for (auto& request: requests) {
        auto emptyResponse = [&request]() {
            if (!request.callback) {
                return;
            }
            request.callback(std::vector<unsigned char>{}, 0, 0,
                    fgviewer::DebugServer::PixelDataFormat::RGBA,
                    fgviewer::DebugServer::FormatInfo{});
        };

        if (request.viewId != viewId) {
            remainingRequests.push_back(std::move(request));
            continue;
        }

        FrameGraphId<FrameGraphTexture> fgTexture =
                fg.getTextureByIdName(request.id, request.name.c_str());
        if (!fgTexture.isInitialized()) {
            utils::slog.e << "[fgviewer] Requested texture id " << request.id << " "
                       << request.name.c_str() << " not found in FrameGraph."
                       << "not found in FrameGraph." << utils::io::endl;

            // Dump all resources for debugging
            utils::slog.e << "[fgviewer] Dumping all FrameGraph resources:" << utils::io::endl;
            fg.dumpResources();
            emptyResponse();
            continue;
        }

        FrameGraphTexture::Descriptor const& texDescOriginal = fg.getDescriptor(fgTexture);
        utils::CString const requestName = request.name;
        std::string_view const requestNameView{ requestName.data(), requestName.length() };
        bool const isViewRenderTarget = requestNameView == "viewRenderTarget";

        // TODO: mip or multi-layer readback is not supported for fgviewer.
        bool const isMip = requestNameView.find("mip") != std::string_view::npos ||
                           requestNameView.find("Layer") != std::string_view::npos;

        if (isMip) {
            utils::slog.w << "[fgviewer] WARNING: Readback for subresources (" << requestName.c_str()
                         << " ) is not supported." << utils::io::endl;
            emptyResponse();
            continue;
        }

        if (texDescOriginal.samples > 1 && !isViewRenderTarget) {
            PostProcessManager& ppm = mPostProcessManager;
            fgTexture = ppm.resolve(fg, "Resolved Monitor", fgTexture, { .levels = 1 },
                    utils::CString(fgviewer::RESOLVED_MONITOR_PASS_NAME));
        }

        FrameGraphTexture::Descriptor const& texDesc = fg.getDescriptor(fgTexture);
        bool const isDepth =
                backend::isDepthFormat(texDesc.format) || backend::isStencilFormat(texDesc.format);

        struct ReadbackPassData {
            FrameGraphId<FrameGraphTexture> texture;
        };

        fg.addPass<ReadbackPassData>(
                fgviewer::READBACK_PASS_NAME,
                [&](FrameGraph::Builder& builder, ReadbackPassData& data) {
                    FrameGraphTexture::Usage usage =
                            isViewRenderTarget ? FrameGraphTexture::Usage::COLOR_ATTACHMENT
                                               : FrameGraphTexture::Usage::SAMPLEABLE;
                    data.texture = builder.read(fgTexture, usage);
                    builder.sideEffect(); // Ensure this pass is not culled
                },
                [request = std::move(request), requestName, isDepth, isViewRenderTarget,
                        viewTarget](FrameGraphResources const& resources,
                        ReadbackPassData const& data, backend::DriverApi& d) {
                    const FrameGraphTexture::Descriptor& desc =
                            resources.getDescriptor(data.texture);

                    backend::TextureFormat texFormat = desc.format;
                    bool const isD24 = (texFormat == backend::TextureFormat::DEPTH24 ||
                                        texFormat == backend::TextureFormat::DEPTH24_STENCIL8);
                    bool const isD16 = (texFormat == backend::TextureFormat::DEPTH16);
                    bool const isD32F = (texFormat == backend::TextureFormat::DEPTH32F ||
                                         texFormat == backend::TextureFormat::DEPTH32F_STENCIL8);

                    backend::PixelBufferDescriptor::PixelDataFormat format =
                            isDepth ? backend::PixelDataFormat::DEPTH_COMPONENT
                                    : backend::PixelDataFormat::RGBA;

                    bool const isFloat = texFormat == backend::TextureFormat::RGB16F ||
                                         texFormat == backend::TextureFormat::RGBA16F ||
                                         texFormat == backend::TextureFormat::R11F_G11F_B10F ||
                                         texFormat == backend::TextureFormat::RGB9_E5;

                    bool const isR8 = texFormat == backend::TextureFormat::R8 ||
                                      texFormat == backend::TextureFormat::R8_SNORM ||
                                      texFormat == backend::TextureFormat::R8UI ||
                                      texFormat == backend::TextureFormat::R8I;

                    backend::PixelBufferDescriptor::PixelDataType type;
                    if (isDepth) {
                        if (isD32F) {
                            type = backend::PixelDataType::FLOAT;
                        } else if (isD16) {
                            type = backend::PixelDataType::USHORT;
                        } else {
                            type = backend::PixelDataType::UINT;
                        }
                    } else {
                        type = isFloat ? backend::PixelDataType::FLOAT
                                       : backend::PixelDataType::UBYTE;
                    }

                    fgviewer::DebugServer::PixelDataFormat const targetFormat =
                            fgviewer::DebugServer::Format::RGBA;

                    const size_t bytesPerPixel =
                            isDepth ? (isD32F ? 4 : (isD16 ? 2 : 4))
                                    : (type == backend::PixelDataType::FLOAT ? 16 : 4);
                    const size_t samples = desc.samples > 1 ? desc.samples : 1;
                    const size_t bufferSize = desc.width * desc.height * bytesPerPixel * samples;
                    fgviewer::DebugServer::PixelBuffer pixelBuffer(bufferSize);
                    void* bufferData = pixelBuffer.data();
                    fgviewer::DebugServer::FormatInfo info{
                        .isDepth = isDepth,
                        .isD16 = isD16,
                        .isD24 = isD24,
                        .isD32F = isD32F,
                        .isFloat = isFloat,
                        .isR8 = isR8,
                        .samples = static_cast<uint8_t>(samples),
                    };

                    struct UserData {
                        size_t width;
                        size_t height;
                        fgviewer::DebugServer::PixelDataFormat targetFormat;
                        Request::Callback callback;
                        fgviewer::DebugServer::PixelBuffer pixelBuffer;
                        fgviewer::DebugServer::FormatInfo info;
                    };

                    backend::PixelBufferDescriptor pbd(
                            bufferData, bufferSize, format, type, 1, 0, 0, 0, 0,
                            [](void* buffer, size_t size, void* user) {
                                std::unique_ptr<UserData> d{ static_cast<UserData*>(user) };
                                auto& callback = d->callback;
                                if (callback) {
                                    callback(std::move(d->pixelBuffer), d->width, d->height,
                                            d->targetFormat, d->info);
                                }
                            },
                            new UserData{ desc.width, desc.height, targetFormat,
                                std::move(request.callback), std::move(pixelBuffer), info });

                    if (isViewRenderTarget) {
                        if (viewTarget) {
                            d.readPixels(viewTarget, 0, 0, desc.width, desc.height, std::move(pbd));
                        } else {
                            utils::slog.e << "[fgviewer] ERROR: Hardware RenderTarget is "
                                          "uninitialized for "
                                       << requestName.c_str() << utils::io::endl;
                            if (request.callback) {
                                request.callback(std::vector<unsigned char>{}, 0, 0,
                                        fgviewer::DebugServer::Format::RGBA,
                                        fgviewer::DebugServer::FormatInfo{});
                            }
                        }
                    } else {
                        auto hwTex = resources.getTexture(data.texture);
                        if (hwTex) {
                            auto srd = resources.getSubResourceDescriptor(data.texture);
                            d.readTexture(hwTex, srd.level, srd.layer, 0, 0, desc.width,
                                    desc.height, std::move(pbd));
                        } else {
                            utils::slog.e << "[fgviewer] ERROR: Hardware Texture is uninitialized for "
                                       << requestName.c_str() << utils::io::endl;
                            if (request.callback) {
                                request.callback(std::vector<unsigned char>{}, 0, 0,
                                        fgviewer::DebugServer::Format::RGBA,
                                        fgviewer::DebugServer::FormatInfo{});
                            }
                        }
                    }
                });
    }

    if (!remainingRequests.empty()) {
        std::unique_lock<utils::Mutex> lock(mReadbackRequestsMutex);
        mReadbackRequests.insert(mReadbackRequests.end(),
                std::make_move_iterator(remainingRequests.begin()),
                std::make_move_iterator(remainingRequests.end()));
    }
}

void FgviewerManager::framegraphUpdated(FrameGraph& fg, FView const& view) {
    auto info = fg.getFrameGraphInfo(view.getName());
    mServer->update(view.getViewHandle(), std::move(info));
}

void FgviewerManager::framegraphExecuted() {
    mServer->tick();
}

void FgviewerManager::requestTextureReadback(fgviewer::ViewHandle viewId, uint32_t id,
        const utils::CString& name,
        Request::Callback&& callback) {
    std::unique_lock<utils::Mutex> lock(mReadbackRequestsMutex);
    mReadbackRequests.emplace_back(Request{ viewId, id, name, std::move(callback) });
}

} // namespace filament

#endif // FILAMENT_ENABLE_FGVIEWER
