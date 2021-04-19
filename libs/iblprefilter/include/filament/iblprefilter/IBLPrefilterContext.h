/*
 * Copyright (C) 2021 The Android Open Source Project
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

#ifndef TNT_IBL_PREFILTER_IBLPREFILTER_H
#define TNT_IBL_PREFILTER_IBLPREFILTER_H

#include <utils/Entity.h>

#include <filament/Texture.h>

namespace filament {
class Engine;
class View;
class Scene;
class Renderer;
class Material;
class MaterialInstance;
class VertexBuffer;
class IndexBuffer;
class Camera;
class Texture;
} // namespace filament

/**
 * IBLPrefilterContext creates and initializes GPU state common to all environment map filters
 * supported. Typically, only one instance per filament Engine of this object needs to exist.
 *
 * Usage Example:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * #include <filament/Engine.h>
 * using namespace filament;
 *
 * Engine* engine = Engine::create();
 *
 * IBLPrefilterContext context(engine);
 * IBLPrefilterContext::SpecularFilter filter(context);
 * Texture* texture = filter(environment_cubemap);
 *
 * IndirectLight* indirectLight = IndirectLight::Builder()
 *     .reflections(texture)
 *     .build(engine);
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
class IBLPrefilterContext {
public:

    /**
     * Creates an IBLPrefilter context.
     * @param engine filament engine to use
     */
    IBLPrefilterContext(filament::Engine& engine);

    /**
     * Destroys all GPU resources created during initialization.
     */
    ~IBLPrefilterContext() noexcept;

    // not copyable
    IBLPrefilterContext(IBLPrefilterContext const&) = delete;
    IBLPrefilterContext& operator=(IBLPrefilterContext const&) = delete;

    // movable
    IBLPrefilterContext(IBLPrefilterContext&& rhs) noexcept;
    IBLPrefilterContext& operator=(IBLPrefilterContext&& rhs);

    // -------------------------------------------------------------------------------------------

    /**
     * SpecularFilter is a GPU based implementation of the specular probe pre-integration filter.
     * An instance of SpecularFilter is needed per filter configuration. A filter configuration
     * contains the filter's kernel and sample count.
     */
    class SpecularFilter {
    public:
        enum class Kernel : uint8_t {
            D_GGX,        // Trowbridge-reitz distribution
        };

        /**
         * Filter configuration.
         */
        struct Config {
            uint16_t sampleCount = 1024u;   //!< filter sample count
            uint8_t levelCount = 5u;        //!< number of roughness levels
            Kernel kernel = Kernel::D_GGX;  //!< filter kernel
        };

        /**
         * Filtering options for the current environment.
         */
        struct Options {
            float hdrLinear = 8.0f;      //!< no HDR compression up to this value
            float hdrMax = 16.0f;        //!< HDR compression between hdrLinear and hdrMax
            bool generateMipmap = true;  //!< set to false if the environment map already has mipmaps
        };

        struct ReflectionsOptions {
            using InternalFormat = filament::Texture::InternalFormat;
            using Usage = filament::Texture::Usage;
            uint16_t size = 256;         //!< output environment texture size. Must be Power-of-two.
            InternalFormat format = InternalFormat::R11F_G11F_B10F; // output environment texture format
            Usage usage = Usage::NONE;  //!< extra usage added to output environment texture
        };

        /**
         * Creates a filter.
         * @param context IBLPrefilterContext to use
         * @param config  Configuration of the filter
         */
        SpecularFilter(IBLPrefilterContext& context, Config config);

        /**
         * Creates a filter with the default configuration.
         * @param context IBLPrefilterContext to use
         */
        explicit SpecularFilter(IBLPrefilterContext& context);

        /**
         * Destroys all GPU resources created during initialization.
         */
        ~SpecularFilter() noexcept;

        SpecularFilter(SpecularFilter const&) = delete;
        SpecularFilter& operator=(SpecularFilter const&) = delete;
        SpecularFilter(SpecularFilter&& rhs) noexcept;
        SpecularFilter& operator=(SpecularFilter&& rhs);

        /**
         * Creates a texture suitable for being used as a reflection map
         * @param options options for the texture creation
         * @return a Texture object owned by the caller.
         */
        filament::Texture* createReflectionsTexture(ReflectionsOptions options);

        /**
         * Generates a prefiltered cubemap.
         * @param options                   Options for this environment
         * @param environmentCubemap        Environment cubemap (input). Can't be null.
         *                                  This cubemap must be SAMPLEABLE.
         * @param outReflectionsTexture     Output prefiltered texture or, if null, it is
         *                                  automatically created. outReflectionsTexture must
         *                                  be a cubemap, it must have at least COLOR_ATTACHMENT and
         *                                  SAMPLEABLE usages and at least the same number of levels
         *                                  than requested by Config.
         * @return returns outReflectionsTexture
         */
        filament::Texture* operator()(Options options,
                filament::Texture const* environmentCubemap,
                filament::Texture* outReflectionsTexture = nullptr);

        /**
         * Generates a prefiltered cubemap with the default options.
         * @param environmentCubemap
         * @param outReflectionsTexture
         * @return
         */
        filament::Texture* operator()(
                filament::Texture const* environmentCubemap,
                filament::Texture* outReflectionsTexture = nullptr);

        // TODO: option for progressive filtering

        // TODO: add a callback for when the processing is done?

    private:
        friend class Instance;
        IBLPrefilterContext& mContext;
        Config mConfig{};
        filament::Texture* mKernelTexture = nullptr;
        float* mKernelWeightArray = nullptr;
        uint32_t mSampleCount = 0u;
        uint8_t mLevelCount = 1u;
    };

private:
    friend class Filter;
    filament::Engine& mEngine;
    filament::Renderer* mRenderer{};
    filament::Scene* mScene{};
    filament::VertexBuffer* mVertexBuffer{};
    filament::IndexBuffer* mIndexBuffer{};
    filament::Camera* mCamera{};
    utils::Entity mFullScreenQuadEntity{};
    utils::Entity mCameraEntity{};
    filament::View* mView{};
    // this one might have to move into Filter
    filament::Material* mMaterial{};
};

#endif //TNT_IBL_PREFILTER_IBLPREFILTER_H
