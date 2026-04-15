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

#include <memory>
#include <tuple>

#include "dawn/common/Mutex.h"
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
        ServerBase(const DawnProcTable& procs) : mProcs(std::make_shared<DawnProcTable>(procs)) {}
        ~ServerBase() override = default;

        Mutex::AutoLock GetGuard() { return Mutex::AutoLock(&mMutex); }

      protected:
        // Proc table may be used by children as well.
        std::shared_ptr<const DawnProcTable> mProcs;

        // Template functions that implement helpers on KnownObjects.
        template <typename T>
        WireResult Get(ObjectId id, Reserved<T>* result) {
            return std::get<KnownObjects<T>>(mKnown).Get(id, result);
        }
        template <typename T>
        WireResult Get(ObjectId id, Known<T>* result) {
            return std::get<KnownObjects<T>>(mKnown).Get(id, result);
        }
        template <typename T>
        WireResult FillReservation(ObjectId id, T handle, Known<T>* known = nullptr) {
            auto result = std::get<KnownObjects<T>>(mKnown).FillReservation(id, handle, known);
            if (result == WireResult::FatalError) {
                Release(handle);
            }
            return result;
        }
        template <typename T>
        WireResult Allocate(Reserved<T>* result,
                            ObjectHandle handler,
                            AllocationState state = AllocationState::Allocated) {
            // Allocations always take the lock because |vector::push_back| may be called which
            // can invalidate pointers.
            auto serverGuard = GetGuard();
            return std::get<KnownObjects<T>>(mKnown).Allocate(result, handler, state);
        }
        template <typename T>
        WireResult Free(ObjectId id, ObjectData<T>* data) {
            // Free always take the lock to ensure that any callback handlers (which always take
            // the lock), i.e. On*Callback functions, cannot race with object deletion.
            auto serverGuard = GetGuard();
            Reserved<T> obj;
            WIRE_TRY(Get(id, &obj));
            *data = std::move(*obj.data);
            std::get<KnownObjects<T>>(mKnown).Free(id);
            return WireResult::Success;
        }

        // Special functions that are currently needed.
        bool IsDeviceKnown(WGPUDevice device) const {
            return std::get<KnownObjects<WGPUDevice>>(mKnown).IsKnown(device);
        }
        std::vector<WGPUDevice> GetAllDeviceHandles() const {
            return std::get<KnownObjects<WGPUDevice>>(mKnown).GetAllHandles();
        }
        std::vector<WGPUInstance> GetAllInstanceHandles() const {
            return std::get<KnownObjects<WGPUInstance>>(mKnown).GetAllHandles();
        }

        template <typename T>
        void Release(T handle) {
            ((*mProcs).*WGPUTraits<T>::Release)(handle);
        }
        void DestroyAllObjects() {
            //* Release devices first to force completion of any async work.
            {
                std::vector<WGPUDevice> handles =
                    std::get<KnownObjects<WGPUDevice>>(mKnown).AcquireAllHandles();
                for (WGPUDevice handle : handles) {
                    Release(handle);
                }
            }
            //* Free all objects when the server is destroyed
            {% for type in by_category["object"] if type.name.get() != "device" %}
                {% set cType = as_cType(type.name) %}
                {
                    std::vector<{{cType}}> handles =
                        std::get<KnownObjects<{{cType}}>>(mKnown).AcquireAllHandles();
                    for ({{cType}} handle : handles) {
                        Release(handle);
                    }
                }
            {% endfor %}
        }

      private:
        // Implementation of the ObjectIdResolver interface
        {% for type in by_category["object"] %}
            {% set cType = as_cType(type.name) %}
            WireResult GetFromId(ObjectId id, {{cType}}* out) const final {
                return std::get<KnownObjects<{{cType}}>>(mKnown).GetNativeHandle(id, out);
            }

            WireResult GetOptionalFromId(ObjectId id, {{cType}}* out) const final {
                if (id == 0) {
                    *out = nullptr;
                    return WireResult::Success;
                }
                return GetFromId(id, out);
            }
        {% endfor %}

        // The list of known IDs for each object type.
        // We use an explicit Mutex to protect these lists instead of MutexProtected because:
        //   1) It allows us to return AutoLock objects to hold the lock across function scopes.
        //   2) It allows us to use |mKnown| when we know we can access it without the lock.
        // To clarify the threading model of the KnownObjects, there is always a "main" thread on
        // the server which is responsible for flushing commands from the client. This thread is
        // the only thread that ever calls |HandleCommandsImpl| which ultimately calls into the
        // command handlers in the generated ServerHandlers.cpp. All other threads that can
        // interact with |mKnown| are async or spontaneous callback threads which always hold the
        // lock for the full duration of their call, see |ForwardToServerHelper| where we force all
        // callbacks to take the lock. On the "main" thread, we only acquire the lock whenever we
        // |Allocate| or |Free|. We need to lock w.r.t other threads for |Allocate| because
        // internally, that can cause a |std::vector::push_back| call which may invalidate all
        // pointers. As a result, we cannot have another thread doing anything during that window.
        // Similarly, for |Free|, we may release the WGPU* backing handle which is also not allowed
        // to race. Reads on the "main" thread are not explicitly synchronized because callbacks
        // are not allowed to allocate or free objects which guarantees that the reads on the
        // "main" thread will be pointer stable. Furthermore, it is actually necessary that we
        // don't hold the lock for reads on the "main" thread because APIs like |Device::Destroy|
        // can cause us to wait on callbacks which as stated above, require the lock to complete.
        // So if we were to hold the lock on the "main" thread during the read and execution of
        // |Device::Destroy|, we could deadlock when we try waiting for callbacks which can't
        // acquire this lock.
        // TODO(https://crbug.com/412761856): Revisit this logic and consider using a rw-lock
        // in conjunction with taking additional refs to "special" objects/function pairs that can
        // cause callbacks to run when called. This would make this logic less specific to the way
        // that the Dawn tests and Chromium uses the wire.
        Mutex mMutex;
        std::tuple<
            {% for type in by_category["object"] %}
                KnownObjects<{{as_cType(type.name)}}>{{ ", " if not loop.last else "" }}
            {% endfor %}
        > mKnown;
    };

}  // namespace dawn::wire::server

#endif  // DAWNWIRE_SERVER_SERVERBASE_AUTOGEN_H_
