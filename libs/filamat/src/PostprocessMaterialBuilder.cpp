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

#include "shaders/ShaderGenerator.h"

#include "GLSLPostProcessor.h"

#include "eiff/ChunkContainer.h"
#include "eiff/DictionaryTextChunk.h"
#include "eiff/DictionarySpirvChunk.h"
#include "eiff/MaterialTextChunk.h"
#include "eiff/MaterialSpirvChunk.h"
#include "eiff/SimpleFieldChunk.h"

#include <vector>

using namespace filament::driver;

namespace filamat {

PostprocessMaterialBuilder& PostprocessMaterialBuilder::postProcessor(PostProcessCallBack callback) {
    mPostprocessorCallback = callback;
    return *this;
}

Package PostprocessMaterialBuilder::build() {
    prepare();
    // Install postprocessor to optimize / compile to Spir-V if necessary.
    // TODO: remove the postProcessor functionality, since it isn't being used by the outside world.
    using namespace std::placeholders;
    GLSLPostProcessor postProcessor(mOptimization, mPrintShaders);
    this->postProcessor(std::bind(&GLSLPostProcessor::process, postProcessor, _1, _2, _3, _4, _5));

    // Create chunk tree.
    ChunkContainer container;

    SimpleFieldChunk<uint32_t> version(ChunkType::PostProcessVersion,1);
    container.addChild(&version);

    std::vector<TextEntry> glslEntries;
    std::vector<SpirvEntry> spirvEntries;
    LineDictionary glslDictionary;
    BlobDictionary spirvDictionary;
    std::vector<uint32_t> spirv;

    // Populate a SamplerBindingMap for the sole purpose of finding where the post-process bindings
    // live within the global namespace of samplers.
    filament::SamplerBindingMap samplerBindingMap;
    samplerBindingMap.populate();
    const uint8_t firstSampler =
            samplerBindingMap.getBlockOffset(filament::BindingPoints::POST_PROCESS);
    bool errorOccured = false;
    for (const auto& params : mCodeGenPermutations) {
        const ShaderModel shaderModel = ShaderModel(params.shaderModel);
        const TargetApi targetApi = params.targetApi;
        const TargetApi codeGenTargetApi = params.codeGenTargetApi;
        std::vector<uint32_t>* pSpirv = (targetApi == TargetApi::VULKAN) ? &spirv : nullptr;

        TextEntry glslEntry;
        SpirvEntry spirvEntry;

        glslEntry.shaderModel = static_cast<uint8_t>(params.shaderModel);
        spirvEntry.shaderModel = static_cast<uint8_t>(params.shaderModel);

        for (size_t k = 0; k < filament::POST_PROCESS_STAGES_COUNT; k++) {
            glslEntry.variant = static_cast<uint8_t>(k);
            spirvEntry.variant = static_cast<uint8_t>(k);

            // Vertex Shader
            std::string vs = ShaderPostProcessGenerator::createPostProcessVertexProgram(
                    shaderModel, targetApi, codeGenTargetApi,
                    filament::PostProcessStage(k), firstSampler);

            if (mPostprocessorCallback != nullptr) {
                bool ok = mPostprocessorCallback(vs, filament::driver::ShaderType::VERTEX,
                        shaderModel, &vs, pSpirv);
                if (!ok) {
                    // An error occured while postProcessing, aborting.
                    errorOccured = true;
                    break;
                }
            }

            if (targetApi == TargetApi::OPENGL) {
                glslEntry.stage = filament::driver::ShaderType::VERTEX;
                glslEntry.shaderSize = vs.size();
                glslEntry.shader = (char*)malloc(glslEntry.shaderSize + 1);
                strcpy(glslEntry.shader, vs.c_str());
                glslDictionary.addText(glslEntry.shader);
                glslEntries.push_back(glslEntry);
            }
            if (targetApi == TargetApi::VULKAN) {
                spirvEntry.stage = filament::driver::ShaderType::VERTEX;
                spirvEntry.dictionaryIndex = spirvDictionary.addBlob(spirv);
                spirv.clear();
                spirvEntries.push_back(spirvEntry);
            }

            // Fragment Shader
            std::string fs = ShaderPostProcessGenerator::createPostProcessFragmentProgram(
                    shaderModel, targetApi, codeGenTargetApi,
                    filament::PostProcessStage(k), firstSampler);
            if (mPostprocessorCallback != nullptr) {
                bool ok = mPostprocessorCallback(fs, filament::driver::ShaderType::FRAGMENT,
                        shaderModel, &fs, pSpirv);
                if (!ok) {
                    // An error occured while postProcessing, aborting.
                    errorOccured = true;
                    break;
                }
            }
            if (targetApi == TargetApi::OPENGL) {
                glslEntry.stage = filament::driver::ShaderType::FRAGMENT;
                glslEntry.shaderSize = fs.size();
                glslEntry.shader = (char*)malloc(glslEntry.shaderSize + 1);
                strcpy(glslEntry.shader, fs.c_str());
                glslDictionary.addText(glslEntry.shader);
                glslEntries.push_back(glslEntry);
            }
            if (targetApi == TargetApi::VULKAN) {
                spirvEntry.stage = filament::driver::ShaderType::FRAGMENT;
                spirvEntry.dictionaryIndex = spirvDictionary.addBlob(spirv);
                spirv.clear();
                spirvEntries.push_back(spirvEntry);
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
    return package;
}

} // namespace filamat
