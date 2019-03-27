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

#include "GLSLPostProcessor.h"

#include "eiff/ChunkContainer.h"
#include "eiff/DictionarySpirvChunk.h"
#include "eiff/DictionaryTextChunk.h"
#include "eiff/MaterialSpirvChunk.h"
#include "eiff/MaterialTextChunk.h"
#include "eiff/SimpleFieldChunk.h"
#include "sca/GLSLTools.h"

#include <vector>

using namespace filament::backend;

namespace filamat {

Package PostprocessMaterialBuilder::build() {
    prepare();

    // Create a postprocessor to optimize / compile to Spir-V if necessary.
    GLSLPostProcessor postProcessor(mOptimization, mPrintShaders);

    // Create chunk tree.
    ChunkContainer container;

    SimpleFieldChunk<uint32_t> version(ChunkType::PostProcessVersion, filament::MATERIAL_VERSION);
    container.addChild(&version);

    std::vector<TextEntry> glslEntries;
    std::vector<SpirvEntry> spirvEntries;
    std::vector<TextEntry> metalEntries;
    LineDictionary glslDictionary;
    BlobDictionary spirvDictionary;
    LineDictionary metalDictionary;
    std::vector<uint32_t> spirv;
    std::string msl;

    bool errorOccured = false;

    for (const auto& params : mCodeGenPermutations) {
        const ShaderModel shaderModel = ShaderModel(params.shaderModel);
        const TargetApi targetApi = params.targetApi;
        const TargetLanguage targetLanguage = params.targetLanguage;

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
            std::string vs = ShaderPostProcessGenerator::createPostProcessVertexProgram(
                    shaderModel, targetApi, targetLanguage,
                    filament::PostProcessStage(k), firstSampler);

            bool ok = postProcessor.process(vs, filament::backend::ShaderType::VERTEX, shaderModel,
                    &vs, pSpirv, pMsl);
            if (!ok) {
                // An error occured while postProcessing, aborting.
                errorOccured = true;
                break;
            }

            if (targetApi == TargetApi::OPENGL) {
                glslEntry.stage = filament::backend::ShaderType::VERTEX;
                glslEntry.shaderSize = vs.size();
                glslEntry.shader = (char*)malloc(glslEntry.shaderSize + 1);
                strcpy(glslEntry.shader, vs.c_str());
                glslDictionary.addText(glslEntry.shader);
                glslEntries.push_back(glslEntry);
            }

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
                metalEntry.shaderSize = msl.length();
                metalEntry.shader = (char*)malloc(metalEntry.shaderSize + 1);
                strcpy(metalEntry.shader, msl.c_str());
                spirv.clear();
                msl.clear();
                metalDictionary.addText(metalEntry.shader);
                metalEntries.push_back(metalEntry);
            }

            // Fragment Shader
            std::string fs = ShaderPostProcessGenerator::createPostProcessFragmentProgram(
                    shaderModel, targetApi, targetLanguage,
                    filament::PostProcessStage(k), firstSampler);

            ok = postProcessor.process(fs, filament::backend::ShaderType::FRAGMENT, shaderModel, &fs,
                    pSpirv, pMsl);
            if (!ok) {
                // An error occured while postProcessing, aborting.
                errorOccured = true;
                break;
            }

            if (targetApi == TargetApi::OPENGL) {
                glslEntry.stage = filament::backend::ShaderType::FRAGMENT;
                glslEntry.shaderSize = fs.size();
                glslEntry.shader = (char*) malloc(glslEntry.shaderSize + 1);
                strcpy(glslEntry.shader, fs.c_str());
                glslDictionary.addText(glslEntry.shader);
                glslEntries.push_back(glslEntry);
            }

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
                metalEntry.shaderSize = msl.length();
                metalEntry.shader = (char*)malloc(metalEntry.shaderSize + 1);
                strcpy(metalEntry.shader, msl.c_str());
                spirv.clear();
                msl.clear();
                metalDictionary.addText(metalEntry.shader);
                metalEntries.push_back(metalEntry);
            }
        }
    }

    // Emit GLSL chunks
    DictionaryTextChunk dicGlslChunk(glslDictionary, ChunkType::DictionaryGlsl);
    MaterialTextChunk glslChunk(glslEntries, glslDictionary, ChunkType::MaterialGlsl);
    if (!glslEntries.empty()) {
        container.addChild(&dicGlslChunk);
        container.addChild(&glslChunk);
    }

    // Emit SPIRV chunks
    DictionarySpirvChunk dicSpirvChunk(spirvDictionary);
    MaterialSpirvChunk spirvChunk(spirvEntries);
    if (!spirvEntries.empty()) {
        container.addChild(&dicSpirvChunk);
        container.addChild(&spirvChunk);
    }

    // Emit Metal chunks
    filamat::DictionaryTextChunk dicMetalChunk(metalDictionary, ChunkType::DictionaryMetal);
    MaterialTextChunk metalChunk(metalEntries, metalDictionary, ChunkType::MaterialMetal);
    if (!metalEntries.empty()) {
        container.addChild(&dicMetalChunk);
        container.addChild(&metalChunk);
    }

    // Flatten all chunks in the container into a Package.
    size_t packageSize = container.getSize();
    Package package(packageSize);
    Flattener f(package);
    container.flatten(f);
    package.setValid(!errorOccured);

    // Free all shaders that were created earlier.
    for (TextEntry entry : glslEntries) {
        free(entry.shader);
    }
    for (TextEntry entry : metalEntries) {
        free(entry.shader);
    }
    return package;
}

} // namespace filamat
