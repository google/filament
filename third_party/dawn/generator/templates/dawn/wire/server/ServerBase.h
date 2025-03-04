//* Copyright 2019 The Dawn & Tint Authors
//*
//* Redistribution and use in source and binary forms, with or without
//* modification, are permitted provided that the following conditions are met:
//*
//* 1. Redistributions of source code must retain the above copyright notice, this
//*    list of conditions and the following disclaimer.
//*
//* 2. Redistributions in binary form must reproduce the above copyright notice,
//*    this list of conditions and the following disclaimer in the documentation
//*    and/or other materials provided with the distribution.
//*
//* 3. Neither the name of the copyright holder nor the names of its
//*    contributors may be used to endorse or promote products derived from
//*    this software without specific prior written permission.
//*
//* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef DAWNWIRE_SERVER_SERVERBASE_AUTOGEN_H_
#define DAWNWIRE_SERVER_SERVERBASE_AUTOGEN_H_

#include <tuple>

#include "dawn/dawn_proc_table.h"
#include "dawn/wire/ChunkedCommandHandler.h"
#include "dawn/wire/Wire.h"
#include "dawn/wire/WireCmd_autogen.h"
#include "dawn/wire/WireDeserializeAllocator.h"
#include "dawn/wire/server/ObjectStorage.h"
#include "dawn/wire/server/WGPUTraits_autogen.h"

namespace dawn::wire::server {

    class ServerBase : public ChunkedCommandHandler, public ObjectIdResolver {
      public:
        ServerBase() = default;
        ~ServerBase() override = default;

      protected:
        template <typename T>
        const KnownObjects<T>& Objects() const {
            return std::get<KnownObjects<T>>(mKnown);
        }
        template <typename T>
        KnownObjects<T>& Objects() {
            return std::get<KnownObjects<T>>(mKnown);
        }

        template <typename T>
        void Release(const DawnProcTable& procs, T handle) {
            (procs.*WGPUTraits<T>::Release)(handle);
        }

        void DestroyAllObjects(const DawnProcTable& procs) {
            //* Release devices first to force completion of any async work.
            {
                std::vector<WGPUDevice> handles = Objects<WGPUDevice>().AcquireAllHandles();
                for (WGPUDevice handle : handles) {
                    Release(procs, handle);
                }
            }
            //* Free all objects when the server is destroyed
            {% for type in by_category["object"] if type.name.get() != "device" %}
                {% set cType = as_cType(type.name) %}
                {
                    std::vector<{{cType}}> handles = Objects<{{cType}}>().AcquireAllHandles();
                    for ({{cType}} handle : handles) {
                        Release(procs, handle);
                    }
                }
            {% endfor %}
        }

      private:
        // Implementation of the ObjectIdResolver interface
        {% for type in by_category["object"] %}
            {% set cType = as_cType(type.name) %}
            WireResult GetFromId(ObjectId id, {{cType}}* out) const final {
                return Objects<{{cType}}>().GetNativeHandle(id, out);
            }

            WireResult GetOptionalFromId(ObjectId id, {{cType}}* out) const final {
                if (id == 0) {
                    *out = nullptr;
                    return WireResult::Success;
                }
                return GetFromId(id, out);
            }
        {% endfor %}

        //* The list of known IDs for each object type.
        std::tuple<
            {% for type in by_category["object"] %}
                KnownObjects<{{as_cType(type.name)}}>{{ ", " if not loop.last else "" }}
            {% endfor %}
        > mKnown;
    };

}  // namespace dawn::wire::server

#endif  // DAWNWIRE_SERVER_SERVERBASE_AUTOGEN_H_
