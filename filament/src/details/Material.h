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

#include "ds/DescriptorSetLayout.h"

#include <filament/Material.h>
#include <filament/MaterialEnums.h>

#include <private/filament/EngineEnums.h>
#include <private/filament/BufferInterfaceBlock.h>
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
#include <optional>
#include <string_view>

#include <stddef.h>
#include <stdint.h>

#if FILAMENT_ENABLE_MATDBG
#include <matdbg/DebugServer.h>
#endif

namespace filament {

class MaterialParser;

class  FEngine;

class FMaterial : public Material {
public:
    FMaterial(FEngine& engine, const Builder& builder,
            MaterialDefinition const& definition);
    ~FMaterial() noexcept;

    class DefaultMaterialBuilder : public Builder {
    public:
        DefaultMaterialBuilder();
    };


    void terminate(FEngine& engine);

    // return the uniform interface block for this material
    const BufferInterfaceBlock& getUniformInterfaceBlock() const noexcept {
        return mDefinition.uniformInterfaceBlock;
    }

    DescriptorSetLayout const& getPerViewDescriptorSetLayout() const noexcept {
        assert_invariant(mDefinition.materialDomain == MaterialDomain::POST_PROCESS);
        return mDefinition.perViewDescriptorSetLayout;
    }

    DescriptorSetLayout const& getPerViewDescriptorSetLayout(
            Variant const variant, bool const useVsmDescriptorSetLayout) const noexcept;

    // Returns the layout that should be used when this material is bound to the pipeline for the
    // given variant. Shared variants use the Engine's default material's variants, so we should
    // also use the default material's layout.
    DescriptorSetLayout const& getDescriptorSetLayout(Variant variant = {}) const noexcept {
        if (!isSharedVariant(variant)) {
            return mDefinition.descriptorSetLayout;
        }
        FMaterial const* const pDefaultMaterial = mEngine.getDefaultMaterial();
        if (UTILS_UNLIKELY(!pDefaultMaterial)) {
            return mDefinition.descriptorSetLayout;
        }
        return pDefaultMaterial->getDescriptorSetLayout();
    }

    void compile(CompilerPriorityQueue priority,
            UserVariantFilterMask variantSpec,
            backend::CallbackHandler* handler,
            utils::Invocable<void(Material*)>&& callback) noexcept;

    // Creates an instance of this material, specifying the batching mode.
    FMaterialInstance* createInstance(const char* name) const noexcept;

    bool hasParameter(const char* name) const noexcept;

    bool isSampler(const char* name) const noexcept;

    BufferInterfaceBlock::FieldInfo const* reflect(std::string_view name) const noexcept;

    FMaterialInstance const* getDefaultInstance() const noexcept {
        return const_cast<FMaterial*>(this)->getDefaultInstance();
    }

    FMaterialInstance* getDefaultInstance() noexcept;

    FEngine& getEngine() const noexcept  { return mEngine; }

    bool isCached(Variant const variant) const noexcept {
        return bool(mCachedPrograms[variant.key]);
    }

    void invalidate(Variant::type_t variantMask = 0, Variant::type_t variantValue = 0) noexcept;

    // prepareProgram creates the program for the material's given variant at the backend level.
    // Must be called outside of backend render pass.
    // Must be called before getProgram() below.
    void prepareProgram(Variant const variant,
            backend::CompilerPriorityQueue const priorityQueue) const noexcept {
        // prepareProgram() is called for each RenderPrimitive in the scene, so it must be efficient.
        if (UTILS_UNLIKELY(!isCached(variant))) {
            prepareProgramSlow(variant, priorityQueue);
        }
    }

    // getProgram returns the backend program for the material's given variant.
    // Must be called after prepareProgram().
    [[nodiscard]]
    backend::Handle<backend::HwProgram> getProgram(Variant const variant) const noexcept {
#if FILAMENT_ENABLE_MATDBG
        return getProgramWithMATDBG(variant);
#endif
        assert_invariant(mCachedPrograms[variant.key]);
        return mCachedPrograms[variant.key];
    }

    // MaterialInstance::use() binds descriptor sets before drawing. For shared variants,
    // however, the material instance will call useShared() to bind the default material's sets
    // instead.
    // Returns true if this is a shared variant.
    bool useShared(backend::DriverApi& driver, Variant variant) const noexcept {
        if (!isSharedVariant(variant)) {
            return false;
        }
        FMaterial const* const pDefaultMaterial = mEngine.getDefaultMaterial();
        if (UTILS_UNLIKELY(!pDefaultMaterial)) {
            return false;
        }
        FMaterialInstance const* const pDefaultInstance = pDefaultMaterial->getDefaultInstance();
        pDefaultInstance->use(driver, variant);
        return true;
    }

    [[nodiscard]]
    backend::Handle<backend::HwProgram> getProgramWithMATDBG(Variant variant) const noexcept;

    bool isVariantLit() const noexcept { return mDefinition.isVariantLit; }

    const utils::CString& getName() const noexcept { return mDefinition.name; }
    backend::FeatureLevel getFeatureLevel() const noexcept { return mDefinition.featureLevel; }
    backend::RasterState getRasterState() const noexcept  { return mDefinition.rasterState; }
    uint32_t getId() const noexcept { return mMaterialId; }

    UserVariantFilterMask getSupportedVariants() const noexcept {
        return UserVariantFilterMask(UserVariantFilterBit::ALL) & ~mDefinition.variantFilterMask;
    }

    Shading getShading() const noexcept { return mDefinition.shading; }
    Interpolation getInterpolation() const noexcept { return mDefinition.interpolation; }
    BlendingMode getBlendingMode() const noexcept { return mDefinition.blendingMode; }
    VertexDomain getVertexDomain() const noexcept { return mDefinition.vertexDomain; }
    MaterialDomain getMaterialDomain() const noexcept { return mDefinition.materialDomain; }
    CullingMode getCullingMode() const noexcept { return mDefinition.cullingMode; }
    TransparencyMode getTransparencyMode() const noexcept { return mDefinition.transparencyMode; }
    bool isColorWriteEnabled() const noexcept { return mDefinition.rasterState.colorWrite; }
    bool isDepthWriteEnabled() const noexcept { return mDefinition.rasterState.depthWrite; }
    bool isDepthCullingEnabled() const noexcept {
        return mDefinition.rasterState.depthFunc != backend::RasterState::DepthFunc::A;
    }
    bool isDoubleSided() const noexcept { return mDefinition.doubleSided; }
    bool hasDoubleSidedCapability() const noexcept { return mDefinition.doubleSidedCapability; }
    bool isAlphaToCoverageEnabled() const noexcept { return mDefinition.rasterState.alphaToCoverage; }
    float getMaskThreshold() const noexcept { return mDefinition.maskThreshold; }
    bool hasShadowMultiplier() const noexcept { return mDefinition.hasShadowMultiplier; }
    AttributeBitset getRequiredAttributes() const noexcept { return mDefinition.requiredAttributes; }
    RefractionMode getRefractionMode() const noexcept { return mDefinition.refractionMode; }
    RefractionType getRefractionType() const noexcept { return mDefinition.refractionType; }
    ReflectionMode getReflectionMode() const noexcept { return mDefinition.reflectionMode; }

    bool hasSpecularAntiAliasing() const noexcept { return mDefinition.specularAntiAliasing; }
    float getSpecularAntiAliasingVariance() const noexcept { return mDefinition.specularAntiAliasingVariance; }
    float getSpecularAntiAliasingThreshold() const noexcept { return mDefinition.specularAntiAliasingThreshold; }

    backend::descriptor_binding_t getSamplerBinding(
            std::string_view const& name) const;

    const char* getParameterTransformName(std::string_view samplerName) const noexcept;

    bool hasMaterialProperty(Property property) const noexcept {
        return bool(mDefinition.materialProperties & uint64_t(property));
    }

    SamplerInterfaceBlock const& getSamplerInterfaceBlock() const noexcept {
        return mDefinition.samplerInterfaceBlock;
    }

    size_t getParameterCount() const noexcept {
        return mDefinition.uniformInterfaceBlock.getFieldInfoList().size() +
               mDefinition.samplerInterfaceBlock.getSamplerInfoList().size() +
               (mDefinition.subpassInfo.isValid ? 1 : 0);
    }
    size_t getParameters(ParameterInfo* parameters, size_t count) const noexcept;

    uint32_t generateMaterialInstanceId() const noexcept { return mMaterialInstanceId++; }

    void destroyPrograms(FEngine& engine,
            Variant::type_t variantMask = 0,
            Variant::type_t variantValue = 0);

    // return the id of a specialization constant specified by name for this material
    std::optional<uint32_t> getSpecializationConstantId(std::string_view name) const noexcept ;

    // Sets a specialization constant by id. call is no-op if the id is invalid.
    // Return true is the value was changed.
    template<typename T, typename = Builder::is_supported_constant_parameter_t<T>>
    bool setConstant(uint32_t id, T value) noexcept;

    uint8_t getPerViewLayoutIndex() const noexcept {
        return mDefinition.perViewLayoutIndex;
    }

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
    MaterialParser const& getMaterialParser() const noexcept;

    bool hasVariant(Variant variant) const noexcept;
    void prepareProgramSlow(Variant variant,
            CompilerPriorityQueue priorityQueue) const noexcept;
    void getSurfaceProgramSlow(Variant variant,
            CompilerPriorityQueue priorityQueue) const noexcept;
    void getPostProcessProgramSlow(Variant variant,
            CompilerPriorityQueue priorityQueue) const noexcept;
    backend::Program getProgramWithVariants(Variant variant,
            Variant vertexVariant, Variant fragmentVariant) const;

    utils::FixedCapacityVector<backend::Program::SpecializationConstant>
            processSpecializationConstants(Builder const& builder);

    void precacheDepthVariants(FEngine& engine);

    void createAndCacheProgram(backend::Program&& p, Variant variant) const noexcept;

    inline bool isSharedVariant(Variant const variant) const {
        return (mDefinition.materialDomain == MaterialDomain::SURFACE) && !mIsDefaultMaterial &&
               !mDefinition.hasCustomDepthShader && Variant::isValidDepthVariant(variant);
    }

    mutable std::array<backend::Handle<backend::HwProgram>, VARIANT_COUNT> mCachedPrograms;
    MaterialDefinition const& mDefinition;

    bool mIsDefaultMaterial = false;

    // reserve some space to construct the default material instance
    mutable FMaterialInstance* mDefaultMaterialInstance = nullptr;

    // current specialization constants for the HwProgram
    utils::FixedCapacityVector<backend::Program::SpecializationConstant> mSpecializationConstants;

#if FILAMENT_ENABLE_MATDBG
    matdbg::MaterialKey mDebuggerId;
    mutable utils::Mutex mActiveProgramsLock;
    mutable VariantList mActivePrograms;
    mutable utils::Mutex mPendingEditsLock;
    std::unique_ptr<MaterialParser> mPendingEdits;
    std::unique_ptr<MaterialParser> mEditedMaterialParser;
    void setPendingEdits(std::unique_ptr<MaterialParser> pendingEdits) noexcept;
    bool hasPendingEdits() const noexcept;
    void latchPendingEdits() noexcept;
#endif

    FEngine& mEngine;
    const uint32_t mMaterialId;
    mutable uint32_t mMaterialInstanceId = 0;
};


FILAMENT_DOWNCAST(Material)

} // namespace filament

#endif // TNT_FILAMENT_DETAILS_MATERIAL_H
