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

#ifndef TNT_FILAMENT_FG_FRAMEGRAPHPASS_H
#define TNT_FILAMENT_FG_FRAMEGRAPHPASS_H

#include "backend/DriverApiForward.h"

#include "fg/FrameGraphResources.h"

#include <utils/Allocator.h>

namespace filament {

class FrameGraphPassExecutor {
    friend class FrameGraph;
    friend class PassNode;
    friend class RenderPassNode;

protected:
    virtual void execute(FrameGraphResources const& resources, backend::DriverApi& driver) noexcept = 0;

public:
    FrameGraphPassExecutor() noexcept = default;
    virtual ~FrameGraphPassExecutor() noexcept;
    FrameGraphPassExecutor(FrameGraphPassExecutor const&) = delete;
    FrameGraphPassExecutor& operator = (FrameGraphPassExecutor const&) = delete;
};

class FrameGraphPassBase : protected FrameGraphPassExecutor {
    friend class FrameGraph;
    friend class PassNode;
    friend class RenderPassNode;
    PassNode* mNode = nullptr;
    void setNode(PassNode* node) noexcept { mNode = node; }
    PassNode const& getNode() const noexcept { return *mNode; }

public:
    using FrameGraphPassExecutor::FrameGraphPassExecutor;
    ~FrameGraphPassBase() noexcept override;
};

template<typename Data>
class FrameGraphPass : public FrameGraphPassBase {
    friend class FrameGraph;

    // allow our allocators to instantiate us
    template<typename, typename, typename, typename>
    friend class utils::Arena;

    void execute(FrameGraphResources const&, backend::DriverApi&) noexcept override {}

protected:
    FrameGraphPass() = default;
    Data mData;

public:
    Data const& getData() const noexcept { return mData; }
    Data const* operator->() const { return &mData; }
};

template<typename Data, typename Execute>
class FrameGraphPassConcrete : public FrameGraphPass<Data> {
    friend class FrameGraph;

    // allow our allocators to instantiate us
    template<typename, typename, typename, typename>
    friend class utils::Arena;

    explicit FrameGraphPassConcrete(Execute&& execute) noexcept
            : mExecute(std::move(execute)) {
    }

    void execute(FrameGraphResources const& resources, backend::DriverApi& driver) noexcept final {
        mExecute(resources, this->mData, driver);
    }

    Execute mExecute;
};

} // namespace filament

#endif //TNT_FILAMENT_FG_FRAMEGRAPHPASS_H
