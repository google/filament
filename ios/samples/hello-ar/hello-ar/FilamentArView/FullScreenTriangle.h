/*
 * Copyright (C) 2019 The Android Open Source Project
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

#ifndef FullScreenTriangle_h
#define FullScreenTriangle_h

#include <filament/Engine.h>

#include <utils/Entity.h>

#include <math/mat3.h>

using utils::Entity;

class FullScreenTriangle {
public:

    FullScreenTriangle(filament::Engine* engine);
    ~FullScreenTriangle();
    FullScreenTriangle(const FullScreenTriangle&) = delete;
    FullScreenTriangle& operator=(const FullScreenTriangle&) = delete;

    Entity getEntity() const { return mCameraFeedTriangle; }
    void setCameraFeedTexture(void* pixelBufferRef);
    void setCameraFeedTransform(filament::math::mat3f transform);

private:

    void createRenderable();

    filament::Engine* mEngine;

    filament::VertexBuffer* mVertexBuffer;
    filament::IndexBuffer* mIndexBuffer;
    filament::Material* mMaterial;
    filament::MaterialInstance* mMaterialInstance;
    filament::Texture* mCameraFeedTexture;
    Entity mCameraFeedTriangle;

};

#endif /* FullScreenTriangle_h */
