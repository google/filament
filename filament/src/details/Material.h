/*
 * Copyright (C) 2015 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DETAILS_MATERIAL_H
#define TNT_FILAMENT_DETAILS_MATERIAL_H

#include "downcast.h"

#include "details/MaterialInstance.h"

#include <filament/Material.h>
#include <filament/MaterialEnums.h>

#include <private/filament/EngineEnums.h>
#include <private/filament/BufferInterfaceBlock.h>
#include <private/filament/SamplerBindingsInfo.h>
#include <private/filament/SamplerInterfaceBlock.h>
#include <private/filament/SubpassInfo.h>
#include <private/filament/Variant.h>
#include <private/filament/ConstantInfo.h>

#include <backend/CallbackHandler.h>
#include <backend/DriverEnums.h>
#include <backend/Handle.h>
#include <backend/Program.h>

#include <utils/compiler.h>
#include <utils/CString.h>
#include <utils/debug.h>
#include <utils/FixedCapacityVector.h>
#include <utils/Invocable.h>
#include <utils/Mutex.h>

#include <array>
#include <atomic>
#include <memory>
#include <mutex>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <utility>

#include <stddef.h>
#include <stdint.h>

namespace filament {

class MaterialParser;

class  FEngine;

class FMaterial : public Material {
public:
    FMaterial(FEngine& engine, const Material::Builder& builder,
            std::unique_ptr<MaterialParser> materialParser);
    ~FMaterial() noexcept;

    class DefaultMaterialBuilder : public Material::Builder {
    public:
        DefaultMaterialBuilder();
    };


    void terminate(FEngine& engine);

    // return the uniform interface block for this material
    const BufferInterfaceBlock& getUniformInterfaceBlock() const noexcept {
        return mUniformInterfaceBlock;
    }

    // return the uniform interface block for this material
    const SamplerInterfaceBlock& getSamplerInterfaceBlock() const noexcept {
        return mSamplerInterfaceBlock;
    }

    void compile(CompilerPriorityQueue priority,
            UserVariantFilterMask variantFilter,
            backend::CallbackHandler* handler,
            utils::Invocable<void(Material*)>&& callback) noexcept;

    // Create an instance of this material
    FMaterialInstance* createInstance(const char* name) const noexcept;

    bool hasParameter(const char* name) const noexcept;

    bool isSampler(const char* name) const noexcept;

    BufferInterfaceBlock::FieldInfo const* reflect(std::string_view name) const noexcept;

    FMaterialInstance const* getDefaultInstance() const noexcept { return &mDefaultInstance; }
    FMaterialInstance* getDefaultInstance() noexcept { return &mDefaultInstance; }

    FEngine& getEngine() const noexcept  { return mEngine; }

    bool isCached(Variant variant) const noexcept {
        return bool(mCachedPrograms[variant.key]);
    }

    void invalidate(Variant::type_t variantMask = 0, Variant::type_t variantValue = 0) noexcept;

    // prepareProgram creates the program for the material's given variant at the backend level.
    // Must be called outside of backend render pass.
    // Must be called before getProgram() below.
    void prepareProgram(Variant variant,
            backend::CompilerPriorityQueue priorityQueue = CompilerPriorityQueue::HIGH) const noexcept {
        // prepareProgram() is called for each RenderPrimitive in the scene, so it must be efficient.
        if (UTILS_UNLIKELY(!isCached(variant))) {
            prepareProgramSlow(variant, priorityQueue);
        }
    }

    // getProgram returns the backend program for the material's given variant.
    // Must be called after prepareProgram().
    [[nodiscard]] backend::Handle<backend::HwProgram> getProgram(Variant variant) const noexcept {
#if FILAMENT_ENABLE_MATDBG
        assert_invariant((size_t)variant.key < VARIANT_COUNT);
        std::unique_lock<utils::Mutex> lock(mActiveProgramsLock);
        mActivePrograms.set(variant.key);
        lock.unlock();
#endif
        assert_invariant(mCachedPrograms[variant.key]);
        return mCachedPrograms[variant.key];
    }

    bool isVariantLit() const noexcept { return mIsVariantLit; }

    const utils::CString& getName() const noexcept { return mName; }
    backend::FeatureLevel getFeatureLevel() const noexcept { return mFeatureLevel; }
    backend::RasterState getRasterState() const noexcept  { return mRasterState; }
    uint32_t getId() const noexcept { return mMaterialId; }

    UserVariantFilterMask getSupportedVariants() const noexcept {
        return UserVariantFilterMask(UserVariantFilterBit::ALL) & ~mVariantFilterMask;
    }

    Shading getShading() const noexcept { return mShading; }
    Interpolation getInterpolation() const noexcept { return mInterpolation; }
    BlendingMode getBlendingMode() const noexcept { return mBlendingMode; }
    VertexDomain getVertexDomain() const noexcept { return mVertexDomain; }
    MaterialDomain getMaterialDomain() const noexcept { return mMaterialDomain; }
    CullingMode getCullingMode() const noexcept { return mCullingMode; }
    TransparencyMode getTransparencyMode() const noexcept { return mTransparencyMode; }
    bool isColorWriteEnabled() const noexcept { return mRasterState.colorWrite; }
    bool isDepthWriteEnabled() const noexcept { return mRasterState.depthWrite; }
    bool isDepthCullingEnabled() const noexcept {
        return mRasterState.depthFunc != backend::RasterState::DepthFunc::A;
    }
    bool isDoubleSided() const noexcept { return mDoubleSided; }
    bool hasDoubleSidedCapability() const noexcept { return mDoubleSidedCapability; }
    bool isAlphaToCoverageEnabled() const noexcept { return mRasterState.alphaToCoverage; }
    float getMaskThreshold() const noexcept { return mMaskThreshold; }
    bool hasShadowMultiplier() const noexcept { return mHasShadowMultiplier; }
    AttributeBitset getRequiredAttributes() const noexcept { return mRequiredAttributes; }
    RefractionMode getRefractionMode() const noexcept { return mRefractionMode; }
    RefractionType getRefractionType() const noexcept { return mRefractionType; }
    ReflectionMode getReflectionMode() const noexcept { return mReflectionMode; }

    bool hasSpecularAntiAliasing() const noexcept { return mSpecularAntiAliasing; }
    float getSpecularAntiAliasingVariance() const noexcept { return mSpecularAntiAliasingVariance; }
    float getSpecularAntiAliasingThreshold() const noexcept { return mSpecularAntiAliasingThreshold; }

    bool hasMaterialProperty(Property property) const noexcept {
        return bool(mMaterialProperties & uint64_t(property));
    }

    size_t getParameterCount() const noexcept {
        return mUniformInterfaceBlock.getFieldInfoList().size() +
               mSamplerInterfaceBlock.getSamplerInfoList().size() +
               (mSubpassInfo.isValid ? 1 : 0);
    }
    size_t getParameters(ParameterInfo* parameters, size_t count) const noexcept;

    uint32_t generateMaterialInstanceId() const noexcept { return mMaterialInstanceId++; }

    void destroyPrograms(FEngine& engine);

    // return the id of a specialization constant specified by name for this material
    std::optional<uint32_t> getSpecializationConstantId(std::string_view name) const noexcept ;

    // Sets a specialization constant by id. call is no-op if the id is invalid.
    // Return true is the value was changed.
    template<typename T, typename = Builder::is_supported_constant_parameter_t<T>>
    bool setConstant(uint32_t id, T value) noexcept;

#if FILAMENT_ENABLE_MATDBG
    void applyPendingEdits() noexcept;

    /**
     * Callback handlers for the debug server, potentially called from any thread. The userdata
     * argument has the same value that was passed to DebugServer::addMaterial(), which should
     * be an instance of the public-facing Material.
     * @{
     */

    /** Replaces the material package. */
    static void onEditCallback(void* userdata, const utils::CString& name, const void* packageData,
            size_t packageSize);

    /**
     * Returns a list of "active" variants.
     *
     * This works by checking which variants have been accessed since the previous call, then
     * clearing out the internal list.  Note that the active vs inactive status is merely a visual
     * indicator in the matdbg UI, and that it gets updated about every second.
     */
    static void onQueryCallback(void* userdata, VariantList* pActiveVariants);

    void checkProgramEdits() noexcept {
        if (UTILS_UNLIKELY(hasPendingEdits())) {
            applyPendingEdits();
        }
    }

    /** @}*/
#endif

private:
    bool hasVariant(Variant variant) const noexcept;
    void prepareProgramSlow(Variant variant,
            CompilerPriorityQueue priorityQueue) const noexcept;
    void getSurfaceProgramSlow(Variant variant,
            CompilerPriorityQueue priorityQueue) const noexcept;
    void getPostProcessProgramSlow(Variant variant,
            CompilerPriorityQueue priorityQueue) const noexcept;
    backend::Program getProgramWithVariants(Variant variant,
            Variant vertexVariant, Variant fragmentVariant) const noexcept;

    void createAndCacheProgram(backend::Program&& p, Variant variant) const noexcept;

    // try to order by frequency of use
    mutable std::array<backend::Handle<backend::HwProgram>, VARIANT_COUNT> mCachedPrograms;

    backend::RasterState mRasterState;
    TransparencyMode mTransparencyMode = TransparencyMode::DEFAULT;
    bool mIsVariantLit = false;
    backend::FeatureLevel mFeatureLevel = backend::FeatureLevel::FEATURE_LEVEL_1;
    Shading mShading = Shading::UNLIT;

    BlendingMode mBlendingMode = BlendingMode::OPAQUE;
    std::array<backend::BlendFunction, 4> mCustomBlendFunctions = {};
    Interpolation mInterpolation = Interpolation::SMOOTH;
    VertexDomain mVertexDomain = VertexDomain::OBJECT;
    MaterialDomain mMaterialDomain = MaterialDomain::SURFACE;
    CullingMode mCullingMode = CullingMode::NONE;
    AttributeBitset mRequiredAttributes;
    UserVariantFilterMask mVariantFilterMask = 0;
    RefractionMode mRefractionMode = RefractionMode::NONE;
    RefractionType mRefractionType = RefractionType::SOLID;
    ReflectionMode mReflectionMode = ReflectionMode::DEFAULT;
    uint64_t mMaterialProperties = 0;

    float mMaskThreshold = 0.4f;
    float mSpecularAntiAliasingVariance = 0.0f;
    float mSpecularAntiAliasingThreshold = 0.0f;

    bool mDoubleSided = false;
    bool mDoubleSidedCapability = false;
    bool mHasShadowMultiplier = false;
    bool mHasCustomDepthShader = false;
    bool mIsDefaultMaterial = false;
    bool mSpecularAntiAliasing = false;

    FMaterialInstance mDefaultInstance;
    SamplerInterfaceBlock mSamplerInterfaceBlock;
    BufferInterfaceBlock mUniformInterfaceBlock;
    SubpassInfo mSubpassInfo;
    utils::FixedCapacityVector<std::pair<utils::CString, uint8_t>> mUniformBlockBindings;
    utils::FixedCapacityVector<Variant> mDepthVariants; // only populated with default material

    using BindingUniformInfoContainer = utils::FixedCapacityVector<
            std::pair<filament::UniformBindingPoints, backend::Program::UniformInfo>>;

    BindingUniformInfoContainer mBindingUniformInfo;

    using AttributeInfoContainer = utils::FixedCapacityVector<std::pair<utils::CString, uint8_t>>;

    AttributeInfoContainer mAttributeInfo;

    SamplerGroupBindingInfoList mSamplerGroupBindingInfoList;
    SamplerBindingToNameMap mSamplerBindingToNameMap;
    // Constants defined by this Material
    utils::FixedCapacityVector<MaterialConstant> mMaterialConstants;
    // A map from the Constant name to the mMaterialConstant index
    std::unordered_map<std::string_view, uint32_t> mSpecializationConstantsNameToIndex;
    // current specialization constants for the HwProgram
    utils::FixedCapacityVector<backend::Program::SpecializationConstant> mSpecializationConstants;

#if FILAMENT_ENABLE_MATDBG
    matdbg::MaterialKey mDebuggerId;
    mutable utils::Mutex mActiveProgramsLock;
    mutable VariantList mActivePrograms;
    mutable utils::Mutex mPendingEditsLock;
    std::unique_ptr<MaterialParser> mPendingEdits;
    void setPendingEdits(std::unique_ptr<MaterialParser> pendingEdits) noexcept {
        std::lock_guard lock(mPendingEditsLock);
        std::swap(pendingEdits, mPendingEdits);
    }
    bool hasPendingEdits() noexcept {
        std::lock_guard lock(mPendingEditsLock);
        return (bool)mPendingEdits;
    }
    void latchPendingEdits() noexcept {
        std::lock_guard lock(mPendingEditsLock);
        std::swap(mPendingEdits, mMaterialParser);
    }

#endif

    utils::CString mName;
    FEngine& mEngine;
    const uint32_t mMaterialId;
    uint64_t mCacheId = 0;
    mutable uint32_t mMaterialInstanceId = 0;
    std::unique_ptr<MaterialParser> mMaterialParser;
};


FILAMENT_DOWNCAST(Material)

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_MATERIAL_H
