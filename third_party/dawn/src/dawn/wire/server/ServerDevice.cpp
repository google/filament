// Copyright 2019 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/wire/server/Server.h"

#include "dawn/common/StringViewUtils.h"
#include "dawn/wire/Wire.h"
#include "dawn/wire/WireResult.h"

namespace dawn::wire::server {

void Server::OnUncapturedError(ObjectHandle device, WGPUErrorType type, WGPUStringView message) {
    ReturnDeviceUncapturedErrorCallbackCmd cmd;
    cmd.device = device;
    cmd.type = type;
    cmd.message = message;

    SerializeCommand(cmd);
}

void Server::OnDeviceLost(DeviceLostUserdata* userdata,
                          WGPUDevice const* device,
                          WGPUDeviceLostReason reason,
                          WGPUStringView message) {
    ReturnDeviceLostCallbackCmd cmd;
    cmd.eventManager = userdata->eventManager;
    cmd.future = userdata->future;
    cmd.reason = reason;
    cmd.message = message;

    SerializeCommand(cmd);
}

void Server::OnLogging(ObjectHandle device, WGPULoggingType type, WGPUStringView message) {
    ReturnDeviceLoggingCallbackCmd cmd;
    cmd.device = device;
    cmd.type = type;
    cmd.message = message;

    SerializeCommand(cmd);
}

WireResult Server::DoDevicePopErrorScope(Known<WGPUDevice> device,
                                         ObjectHandle eventManager,
                                         WGPUFuture future) {
    auto userdata = MakeUserdata<ErrorScopeUserdata>();
    userdata->device = device.AsHandle();
    userdata->eventManager = eventManager;
    userdata->future = future;

    mProcs.devicePopErrorScope(device->handle, {nullptr, WGPUCallbackMode_AllowProcessEvents,
                                                ForwardToServer2<&Server::OnDevicePopErrorScope>,
                                                userdata.release(), nullptr});
    return WireResult::Success;
}

void Server::OnDevicePopErrorScope(ErrorScopeUserdata* userdata,
                                   WGPUPopErrorScopeStatus status,
                                   WGPUErrorType type,
                                   WGPUStringView message) {
    ReturnDevicePopErrorScopeCallbackCmd cmd;
    cmd.eventManager = userdata->eventManager;
    cmd.future = userdata->future;
    cmd.status = status;
    cmd.type = type;
    cmd.message = message;

    SerializeCommand(cmd);
}

WireResult Server::DoDeviceCreateComputePipelineAsync(
    Known<WGPUDevice> device,
    ObjectHandle eventManager,
    WGPUFuture future,
    ObjectHandle pipelineObjectHandle,
    const WGPUComputePipelineDescriptor* descriptor) {
    Reserved<WGPUComputePipeline> pipeline;
    WIRE_TRY(Objects<WGPUComputePipeline>().Allocate(&pipeline, pipelineObjectHandle,
                                                     AllocationState::Reserved));

    auto userdata = MakeUserdata<CreatePipelineAsyncUserData>();
    userdata->device = device.AsHandle();
    userdata->eventManager = eventManager;
    userdata->future = future;
    userdata->pipelineObjectID = pipeline.id;

    mProcs.deviceCreateComputePipelineAsync(
        device->handle, descriptor,
        {nullptr, WGPUCallbackMode_AllowProcessEvents,
         ForwardToServer2<&Server::OnCreateComputePipelineAsyncCallback>, userdata.release(),
         nullptr});
    return WireResult::Success;
}

void Server::OnCreateComputePipelineAsyncCallback(CreatePipelineAsyncUserData* data,
                                                  WGPUCreatePipelineAsyncStatus status,
                                                  WGPUComputePipeline pipeline,
                                                  WGPUStringView message) {
    ReturnDeviceCreateComputePipelineAsyncCallbackCmd cmd;
    cmd.eventManager = data->eventManager;
    cmd.future = data->future;
    cmd.status = status;
    cmd.message = message;

    if (status == WGPUCreatePipelineAsyncStatus_Success &&
        FillReservation(data->pipelineObjectID, pipeline) == WireResult::FatalError) {
        cmd.status = WGPUCreatePipelineAsyncStatus_CallbackCancelled;
        cmd.message = ToOutputStringView("Destroyed before request was fulfilled.");
    }
    SerializeCommand(cmd);
}

WireResult Server::DoDeviceCreateRenderPipelineAsync(
    Known<WGPUDevice> device,
    ObjectHandle eventManager,
    WGPUFuture future,
    ObjectHandle pipelineObjectHandle,
    const WGPURenderPipelineDescriptor* descriptor) {
    Reserved<WGPURenderPipeline> pipeline;
    WIRE_TRY(Objects<WGPURenderPipeline>().Allocate(&pipeline, pipelineObjectHandle,
                                                    AllocationState::Reserved));

    auto userdata = MakeUserdata<CreatePipelineAsyncUserData>();
    userdata->device = device.AsHandle();
    userdata->eventManager = eventManager;
    userdata->future = future;
    userdata->pipelineObjectID = pipeline.id;

    mProcs.deviceCreateRenderPipelineAsync(
        device->handle, descriptor,
        {nullptr, WGPUCallbackMode_AllowProcessEvents,
         ForwardToServer2<&Server::OnCreateRenderPipelineAsyncCallback>, userdata.release(),
         nullptr});
    return WireResult::Success;
}

void Server::OnCreateRenderPipelineAsyncCallback(CreatePipelineAsyncUserData* data,
                                                 WGPUCreatePipelineAsyncStatus status,
                                                 WGPURenderPipeline pipeline,
                                                 WGPUStringView message) {
    ReturnDeviceCreateRenderPipelineAsyncCallbackCmd cmd;
    cmd.eventManager = data->eventManager;
    cmd.future = data->future;
    cmd.status = status;
    cmd.message = message;

    if (status == WGPUCreatePipelineAsyncStatus_Success &&
        FillReservation(data->pipelineObjectID, pipeline) == WireResult::FatalError) {
        cmd.status = WGPUCreatePipelineAsyncStatus_CallbackCancelled;
        cmd.message = ToOutputStringView("Destroyed before request was fulfilled.");
    }
    SerializeCommand(cmd);
}

}  // namespace dawn::wire::server
