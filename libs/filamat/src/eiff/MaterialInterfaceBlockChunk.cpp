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
#include "MaterialInterfaceBlockChunk.h"

#include "filament/MaterialChunkType.h"

using namespace filament;

namespace filamat {

MaterialUniformInterfaceBlockChunk::MaterialUniformInterfaceBlockChunk(UniformInterfaceBlock& uib) :
        Chunk(ChunkType::MaterialUib),
        mUib(uib){
}

void MaterialUniformInterfaceBlockChunk::flatten(Flattener &f) {
    f.writeString(mUib.getName().c_str());
    auto uibFields = mUib.getUniformInfoList();
    f.writeUint64(uibFields.size());
    for (auto uInfo: uibFields) {
        f.writeString(uInfo.name.c_str());
        f.writeUint64(uInfo.size);
        f.writeUint8(static_cast<uint8_t>(uInfo.type));
        f.writeUint8(static_cast<uint8_t>(uInfo.precision));
    }
}

MaterialSamplerInterfaceBlockChunk::MaterialSamplerInterfaceBlockChunk(SamplerInterfaceBlock& sib) :
        Chunk(ChunkType::MaterialSib),
        mSib(sib) {
}

void MaterialSamplerInterfaceBlockChunk::flatten(Flattener &f) {
    f.writeString(mSib.getName().c_str());
    auto sibFields = mSib.getSamplerInfoList();
    f.writeUint64(sibFields.size());
    for (auto sInfo: sibFields) {
        f.writeString(sInfo.name.c_str());
        f.writeUint8(static_cast<uint8_t>(sInfo.type));
        f.writeUint8(static_cast<uint8_t>(sInfo.format));
        f.writeUint8(static_cast<uint8_t>(sInfo.precision));
        f.writeBool(sInfo.multisample);
    }
}

MaterialSubpassInterfaceBlockChunk::MaterialSubpassInterfaceBlockChunk(SubpassInfo& subpass) :
        Chunk(ChunkType::MaterialSubpass),
        mSubpass(subpass) {
}

void MaterialSubpassInterfaceBlockChunk::flatten(Flattener &f) {
    f.writeString(mSubpass.block.c_str());
    f.writeUint64(mSubpass.isValid ? 1 : 0);   // only ever a single subpass for now
    if (mSubpass.isValid) {
        f.writeString(mSubpass.name.c_str());
        f.writeUint8(static_cast<uint8_t>(mSubpass.type));
        f.writeUint8(static_cast<uint8_t>(mSubpass.format));
        f.writeUint8(static_cast<uint8_t>(mSubpass.precision));
        f.writeUint8(static_cast<uint8_t>(mSubpass.attachmentIndex));
        f.writeUint8(static_cast<uint8_t>(mSubpass.binding));
    }
}

}
