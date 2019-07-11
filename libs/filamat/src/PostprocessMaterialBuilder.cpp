/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include "filamat/PostprocessMaterialBuilder.h"

#include <filamat/Package.h>

#include <private/filament/SibGenerator.h>

#include "shaders/ShaderGenerator.h"

#include "eiff/ChunkContainer.h"
#include "eiff/DictionarySpirvChunk.h"
#include "eiff/DictionaryTextChunk.h"
#include "eiff/MaterialSpirvChunk.h"
#include "eiff/MaterialTextChunk.h"
#include "eiff/SimpleFieldChunk.h"

#ifndef FILAMAT_LITE
#include "GLSLPostProcessor.h"
#include "sca/GLSLTools.h"
#endif

#include <vector>

using namespace filament::backend;

namespace filamat {

inline void assertSingleTargetApi(MaterialBuilderBase::TargetApi api) {
    // Assert that a single bit is set.
    uint8_t bits = (uint8_t) api;
    assert(bits && !(bits & bits - 1));
}

Package PostprocessMaterialBuilder::build() {
    prepare();

    // Create a postprocessor to optimize / compile to Spir-V if necessary.
#ifndef FILAMAT_LITE
    GLSLPostProcessor postProcessor(mOptimization, mPrintShaders);
#endif

    // Create chunk tree.
    ChunkContainer container;

    container.addSimpleChild<uint32_t>(ChunkType::PostProcessVersion, filament::MATERIAL_VERSION);

    std::vector<TextEntry> glslEntries;
    std::vector<SpirvEntry> spirvEntries;
    std::vector<TextEntry> metalEntries;
    LineDictionary glslDictionary;
#ifndef FILAMAT_LITE
    BlobDictionary spirvDictionary;
    LineDictionary metalDictionary;
#endif
    std::vector<uint32_t> spirv;
    std::string msl;

    bool errorOccured = false;

    for (const auto& params : mCodeGenPermutations) {
        const ShaderModel shaderModel = ShaderModel(params.shaderModel);
        const TargetApi targetApi = params.targetApi;
        const TargetLanguage targetLanguage = params.targetLanguage;

        assertSingleTargetApi(targetApi);

        // Populate a SamplerBindingMap for the sole purpose of finding where the post-process bindings
        // live within the global namespace of samplers.
        filament::SamplerBindingMap samplerBindingMap;
        samplerBindingMap.populate();
        const uint8_t firstSampler =
                samplerBindingMap.getBlockOffset(filament::BindingPoints::POST_PROCESS);

        // Metal Shading Language is cross-compiled from Vulkan.
        const bool targetApiNeedsSpirv =
                (targetApi == TargetApi::VULKAN || targetApi == TargetApi::METAL);
        const bool targetApiNeedsMsl = targetApi == TargetApi::METAL;
        std::vector<uint32_t>* pSpirv = targetApiNeedsSpirv ? &spirv : nullptr;
        std::string* pMsl = targetApiNeedsMsl ? &msl : nullptr;

        TextEntry glslEntry;
        SpirvEntry spirvEntry;
        TextEntry metalEntry;

        glslEntry.shaderModel = static_cast<uint8_t>(params.shaderModel);
        spirvEntry.shaderModel = static_cast<uint8_t>(params.shaderModel);
        metalEntry.shaderModel = static_cast<uint8_t>(params.shaderModel);

        for (size_t k = 0; k < filament::POST_PROCESS_STAGES_COUNT; k++) {
            glslEntry.variant = static_cast<uint8_t>(k);
            spirvEntry.variant = static_cast<uint8_t>(k);
            metalEntry.variant = k;

            // Vertex Shader
            std::string vs = ShaderPostProcessGenerator::createPostProcessVertexProgramOld(
                    shaderModel, targetApi, targetLanguage,
                    filament::PostProcessStage(k), firstSampler);

#ifndef FILAMAT_LITE
            bool ok = postProcessor.process(vs, filament::backend::ShaderType::VERTEX, shaderModel,
                    &vs, pSpirv, pMsl);
#else
            bool ok = true;
#endif
            if (!ok) {
                // An error occured while postProcessing, aborting.
                errorOccured = true;
                break;
            }

            if (targetApi == TargetApi::OPENGL) {
                glslEntry.stage = filament::backend::ShaderType::VERTEX;
                glslEntry.shader = vs;
                glslDictionary.addText(glslEntry.shader);
                glslEntries.push_back(glslEntry);
            }

#ifndef FILAMAT_LITE
            if (targetApi == TargetApi::VULKAN) {
                spirvEntry.stage = filament::backend::ShaderType::VERTEX;
                spirvEntry.dictionaryIndex = spirvDictionary.addBlob(spirv);
                spirv.clear();
                spirvEntries.push_back(spirvEntry);
            }
            if (targetApi == TargetApi::METAL) {
                assert(spirv.size() > 0);
                assert(msl.length() > 0);
                metalEntry.stage = filament::backend::ShaderType::VERTEX;
                metalEntry.shader = msl;
                spirv.clear();
                msl.clear();
                metalDictionary.addText(metalEntry.shader);
                metalEntries.push_back(metalEntry);
            }
#endif

            // Fragment Shader
            std::string fs = ShaderPostProcessGenerator::createPostProcessFragmentProgramOld(
                    shaderModel, targetApi, targetLanguage,
                    filament::PostProcessStage(k), firstSampler);

#ifndef FILAMAT_LITE
            ok = postProcessor.process(fs, filament::backend::ShaderType::FRAGMENT, shaderModel, &fs,
                    pSpirv, pMsl);
#else
            ok = true;
#endif
            if (!ok) {
                // An error occured while postProcessing, aborting.
                errorOccured = true;
                break;
            }

            if (targetApi == TargetApi::OPENGL) {
                glslEntry.stage = filament::backend::ShaderType::FRAGMENT;
                glslEntry.shader = fs;
                glslDictionary.addText(glslEntry.shader);
                glslEntries.push_back(glslEntry);
            }

#ifndef FILAMAT_LITE
            if (targetApi == TargetApi::VULKAN) {
                spirvEntry.stage = filament::backend::ShaderType::FRAGMENT;
                spirvEntry.dictionaryIndex = spirvDictionary.addBlob(spirv);
                spirv.clear();
                spirvEntries.push_back(spirvEntry);
            }
            if (targetApi == TargetApi::METAL) {
                assert(spirv.size() > 0);
                assert(msl.length() > 0);
                metalEntry.stage = filament::backend::ShaderType::FRAGMENT;
                metalEntry.shader = msl;
                spirv.clear();
                msl.clear();
                metalDictionary.addText(metalEntry.shader);
                metalEntries.push_back(metalEntry);
            }
#endif
        }
    }

    // Emit GLSL chunks
    if (!glslEntries.empty()) {
        const auto& dictionaryChunk = container.addChild<filamat::DictionaryTextChunk>(
                std::move(glslDictionary), ChunkType::DictionaryGlsl);
        container.addChild<MaterialTextChunk>(std::move(glslEntries),
                dictionaryChunk.getDictionary(), ChunkType::MaterialGlsl);
    }

#ifndef FILAMAT_LITE
    // Emit SPIRV chunks
    if (!spirvEntries.empty()) {
        container.addChild<filamat::DictionarySpirvChunk>(std::move(spirvDictionary));
        container.addChild<MaterialSpirvChunk>(std::move(spirvEntries));
    }

    // Emit Metal chunks
    if (!metalEntries.empty()) {
        const auto& dictionaryChunk = container.addChild<filamat::DictionaryTextChunk>(
                std::move(metalDictionary), ChunkType::DictionaryMetal);
        container.addChild<MaterialTextChunk>(std::move(metalEntries),
                dictionaryChunk.getDictionary(), ChunkType::MaterialMetal);
    }
#endif

    // Flatten all chunks in the container into a Package.
    Package package(container.getSize());
    Flattener f(package);
    container.flatten(f);
    package.setValid(!errorOccured);

    return package;
}

} // namespace filamat
