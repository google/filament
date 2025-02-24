/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "stateless/stateless_validation.h"
#include "generated/enum_flag_bits.h"
#include "generated/dispatch_functions.h"

namespace stateless {

bool Device::manual_PreCallValidateGetMemoryFdKHR(VkDevice device, const VkMemoryGetFdInfoKHR *pGetFdInfo, int *pFd,
                                                  const Context &context) const {
    constexpr auto allowed_types = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT | VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT;
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (0 == (pGetFdInfo->handleType & allowed_types)) {
        skip |= LogError("VUID-VkMemoryGetFdInfoKHR-handleType-00672", pGetFdInfo->memory,
                         error_obj.location.dot(Field::pGetFdInfo).dot(Field::handleType),
                         "(%s) is not one of the supported handle types (%s).",
                         string_VkExternalMemoryHandleTypeFlagBits(pGetFdInfo->handleType),
                         string_VkExternalMemoryHandleTypeFlags(allowed_types).c_str());
    }
    return skip;
}

bool Device::manual_PreCallValidateGetMemoryFdPropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType, int fd,
                                                            VkMemoryFdPropertiesKHR *pMemoryFdProperties,
                                                            const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (fd < 0) {
        skip |= LogError("VUID-vkGetMemoryFdPropertiesKHR-fd-00673", device, error_obj.location.dot(Field::fd),
                         "handle (%d) is not a valid POSIX file descriptor.", fd);
    }
    if (handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT) {
        skip |= LogError("VUID-vkGetMemoryFdPropertiesKHR-handleType-00674", device, error_obj.location.dot(Field::handleType),
                         "(VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT) is not allowed.");
    }
    return skip;
}

bool Device::ValidateExternalSemaphoreHandleType(VkSemaphore semaphore, const char *vuid, const Location &handle_type_loc,
                                                 VkExternalSemaphoreHandleTypeFlagBits handle_type,
                                                 VkExternalSemaphoreHandleTypeFlags allowed_types) const {
    bool skip = false;
    if (0 == (handle_type & allowed_types)) {
        skip |= LogError(vuid, semaphore, handle_type_loc, "%s is not one of the supported handleTypes (%s).",
                         string_VkExternalSemaphoreHandleTypeFlagBits(handle_type),
                         string_VkExternalSemaphoreHandleTypeFlags(allowed_types).c_str());
    }
    return skip;
}

bool Device::ValidateExternalFenceHandleType(VkFence fence, const char *vuid, const Location &handle_type_loc,
                                             VkExternalFenceHandleTypeFlagBits handle_type,
                                             VkExternalFenceHandleTypeFlags allowed_types) const {
    bool skip = false;
    if (0 == (handle_type & allowed_types)) {
        skip |= LogError(vuid, fence, handle_type_loc, "%s is not one of the supported handleTypes (%s).",
                         string_VkExternalFenceHandleTypeFlagBits(handle_type),
                         string_VkExternalFenceHandleTypeFlags(allowed_types).c_str());
    }
    return skip;
}

static constexpr VkExternalSemaphoreHandleTypeFlags kSemFdHandleTypes =
    VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD_BIT | VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT;

bool Device::manual_PreCallValidateGetSemaphoreFdKHR(VkDevice device, const VkSemaphoreGetFdInfoKHR *pGetFdInfo, int *pFd,
                                                     const Context &context) const {
    const auto &error_obj = context.error_obj;
    return ValidateExternalSemaphoreHandleType(pGetFdInfo->semaphore, "VUID-VkSemaphoreGetFdInfoKHR-handleType-01136",
                                               error_obj.location.dot(Field::pGetFdInfo).dot(Field::handleType),
                                               pGetFdInfo->handleType, kSemFdHandleTypes);
}

bool Device::manual_PreCallValidateImportSemaphoreFdKHR(VkDevice device, const VkImportSemaphoreFdInfoKHR *pImportSemaphoreFdInfo,
                                                        const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    const Location info_loc = error_obj.location.dot(Field::pImportSemaphoreFdInfo);
    skip |=
        ValidateExternalSemaphoreHandleType(pImportSemaphoreFdInfo->semaphore, "VUID-VkImportSemaphoreFdInfoKHR-handleType-01143",
                                            info_loc.dot(Field::handleType), pImportSemaphoreFdInfo->handleType, kSemFdHandleTypes);

    if (pImportSemaphoreFdInfo->handleType == VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT &&
        (pImportSemaphoreFdInfo->flags & VK_SEMAPHORE_IMPORT_TEMPORARY_BIT) == 0) {
        skip |= LogError("VUID-VkImportSemaphoreFdInfoKHR-handleType-07307", pImportSemaphoreFdInfo->semaphore,
                         info_loc.dot(Field::handleType),
                         "is VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_SYNC_FD_BIT so"
                         " VK_SEMAPHORE_IMPORT_TEMPORARY_BIT must be set, but flags is %s",
                         string_VkSemaphoreImportFlags(pImportSemaphoreFdInfo->flags).c_str());
    }
    return skip;
}

static constexpr VkExternalFenceHandleTypeFlags kFenceFdHandleTypes =
    VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_FD_BIT | VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT;

bool Device::manual_PreCallValidateGetFenceFdKHR(VkDevice device, const VkFenceGetFdInfoKHR *pGetFdInfo, int *pFd,
                                                 const Context &context) const {
    return ValidateExternalFenceHandleType(pGetFdInfo->fence, "VUID-VkFenceGetFdInfoKHR-handleType-01456",
                                           context.error_obj.location.dot(Field::pGetFdInfo).dot(Field::handleType),
                                           pGetFdInfo->handleType, kFenceFdHandleTypes);
}

bool Device::manual_PreCallValidateImportFenceFdKHR(VkDevice device, const VkImportFenceFdInfoKHR *pImportFenceFdInfo,
                                                    const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    const Location info_loc = error_obj.location.dot(Field::pImportFenceFdInfo);
    skip |= ValidateExternalFenceHandleType(pImportFenceFdInfo->fence, "VUID-VkImportFenceFdInfoKHR-handleType-01464",
                                            info_loc.dot(Field::handleType), pImportFenceFdInfo->handleType, kFenceFdHandleTypes);

    if (pImportFenceFdInfo->handleType == VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT &&
        (pImportFenceFdInfo->flags & VK_FENCE_IMPORT_TEMPORARY_BIT) == 0) {
        skip |= LogError("VUID-VkImportFenceFdInfoKHR-handleType-07306", pImportFenceFdInfo->fence, info_loc.dot(Field::handleType),
                         "is VK_EXTERNAL_FENCE_HANDLE_TYPE_SYNC_FD_BIT so"
                         " VK_FENCE_IMPORT_TEMPORARY_BIT must be set, but flags is %s",
                         string_VkFenceImportFlags(pImportFenceFdInfo->flags).c_str());
    }
    return skip;
}

bool Device::manual_PreCallValidateGetMemoryHostPointerPropertiesEXT(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                                     const void *pHostPointer,
                                                                     VkMemoryHostPointerPropertiesEXT *pMemoryHostPointerProperties,
                                                                     const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (handleType != VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT &&
        handleType != VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_MAPPED_FOREIGN_MEMORY_BIT_EXT) {
        skip |=
            LogError("VUID-vkGetMemoryHostPointerPropertiesEXT-handleType-01752", device, error_obj.location.dot(Field::handleType),
                     "is %s.", string_VkExternalMemoryHandleTypeFlagBits(handleType));
    }

    const VkDeviceSize host_pointer = reinterpret_cast<VkDeviceSize>(pHostPointer);
    if (SafeModulo(host_pointer, phys_dev_ext_props.external_memory_host_props.minImportedHostPointerAlignment) != 0) {
        skip |= LogError("VUID-vkGetMemoryHostPointerPropertiesEXT-pHostPointer-01753", device,
                         error_obj.location.dot(Field::pHostPointer),
                         "(0x%" PRIxLEAST64
                         ") is not aligned "
                         "to minImportedHostPointerAlignment (%" PRIuLEAST64 ")",
                         host_pointer, phys_dev_ext_props.external_memory_host_props.minImportedHostPointerAlignment);
    }

    return skip;
}

#ifdef VK_USE_PLATFORM_WIN32_KHR
bool Device::manual_PreCallValidateGetMemoryWin32HandleKHR(VkDevice device,
                                                           const VkMemoryGetWin32HandleInfoKHR *pGetWin32HandleInfo,
                                                           HANDLE *pHandle, const Context &context) const {
    constexpr VkExternalMemoryHandleTypeFlags nt_handles =
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT | VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_BIT |
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP_BIT | VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE_BIT;
    constexpr VkExternalMemoryHandleTypeFlags global_share_handles =
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT | VK_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_TEXTURE_KMT_BIT;
    bool skip = false;
    if ((pGetWin32HandleInfo->handleType & (nt_handles | global_share_handles)) == 0) {
        skip |= LogError("VUID-VkMemoryGetWin32HandleInfoKHR-handleType-00664", pGetWin32HandleInfo->memory,
                         context.error_obj.location.dot(Field::pGetWin32HandleInfo).dot(Field::handleType),
                         "(%s) is not one of the supported handle types (%s).",
                         string_VkExternalMemoryHandleTypeFlagBits(pGetWin32HandleInfo->handleType),
                         string_VkExternalMemoryHandleTypeFlags(nt_handles | global_share_handles).c_str());
    }
    return skip;
}

bool Device::manual_PreCallValidateGetMemoryWin32HandlePropertiesKHR(VkDevice device, VkExternalMemoryHandleTypeFlagBits handleType,
                                                                     HANDLE handle,
                                                                     VkMemoryWin32HandlePropertiesKHR *pMemoryWin32HandleProperties,
                                                                     const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (handle == NULL || handle == INVALID_HANDLE_VALUE) {
        static_assert(sizeof(HANDLE) == sizeof(uintptr_t));  // to use PRIxPTR for HANDLE formatting
        skip |= LogError("VUID-vkGetMemoryWin32HandlePropertiesKHR-handle-00665", device, error_obj.location.dot(Field::handle),
                         "(0x%" PRIxPTR ") is not a valid Windows handle.", reinterpret_cast<std::uintptr_t>(handle));
    }
    if (handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT ||
        handleType == VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT) {
        skip |=
            LogError("VUID-vkGetMemoryWin32HandlePropertiesKHR-handleType-00666", device, error_obj.location.dot(Field::handleType),
                     "%s is not allowed.", string_VkExternalMemoryHandleTypeFlagBits(handleType));
    }
    return skip;
}

static constexpr VkExternalSemaphoreHandleTypeFlags kSemWin32HandleTypes = VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT |
                                                                           VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT |
                                                                           VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D12_FENCE_BIT;

bool Device::manual_PreCallValidateImportSemaphoreWin32HandleKHR(VkDevice device, const VkImportSemaphoreWin32HandleInfoKHR *info,
                                                                 const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    skip |=
        ValidateExternalSemaphoreHandleType(info->semaphore, "VUID-VkImportSemaphoreWin32HandleInfoKHR-handleType-01140",
                                            error_obj.location.dot(Field::pImportSemaphoreWin32HandleInfo).dot(Field::handleType),
                                            info->handleType, kSemWin32HandleTypes);

    static constexpr auto kNameAllowedTypes =
        VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_BIT | VK_EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D12_FENCE_BIT;
    if ((info->handleType & kNameAllowedTypes) == 0 && info->name) {
        skip |= LogError("VUID-VkImportSemaphoreWin32HandleInfoKHR-handleType-01466", info->semaphore,
                         error_obj.location.dot(Field::pImportSemaphoreWin32HandleInfo).dot(Field::name),
                         "(%p) must be NULL if handleType is %s", reinterpret_cast<const void *>(info->name),
                         string_VkExternalSemaphoreHandleTypeFlagBits(info->handleType));
    }
    if (info->handle && info->name) {
        skip |= LogError("VUID-VkImportSemaphoreWin32HandleInfoKHR-handle-01469", info->semaphore,
                         error_obj.location.dot(Field::pImportSemaphoreWin32HandleInfo),
                         "both handle (%p) and name (%p) are non-NULL", info->handle, reinterpret_cast<const void *>(info->name));
    }
    return skip;
}

bool Device::manual_PreCallValidateGetSemaphoreWin32HandleKHR(VkDevice device, const VkSemaphoreGetWin32HandleInfoKHR *info,
                                                              HANDLE *pHandle, const Context &context) const {
    return ValidateExternalSemaphoreHandleType(info->semaphore, "VUID-VkSemaphoreGetWin32HandleInfoKHR-handleType-01131",
                                               context.error_obj.location.dot(Field::pGetWin32HandleInfo).dot(Field::handleType),
                                               info->handleType, kSemWin32HandleTypes);
}

static constexpr VkExternalFenceHandleTypeFlags kFenceWin32HandleTypes =
    VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT | VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_KMT_BIT;

bool Device::manual_PreCallValidateImportFenceWin32HandleKHR(VkDevice device, const VkImportFenceWin32HandleInfoKHR *info,
                                                             const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    skip |= ValidateExternalFenceHandleType(info->fence, "VUID-VkImportFenceWin32HandleInfoKHR-handleType-01457",
                                            error_obj.location.dot(Field::pImportFenceWin32HandleInfo).dot(Field::handleType),
                                            info->handleType, kFenceWin32HandleTypes);

    static constexpr auto kNameAllowedTypes = VK_EXTERNAL_FENCE_HANDLE_TYPE_OPAQUE_WIN32_BIT;
    if ((info->handleType & kNameAllowedTypes) == 0 && info->name) {
        skip |= LogError("VUID-VkImportFenceWin32HandleInfoKHR-handleType-01459", info->fence,
                         error_obj.location.dot(Field::pImportFenceWin32HandleInfo).dot(Field::name),
                         "(%p) must be NULL if handleType is %s", reinterpret_cast<const void *>(info->name),
                         string_VkExternalFenceHandleTypeFlagBits(info->handleType));
    }
    if (info->handle && info->name) {
        skip |= LogError("VUID-VkImportFenceWin32HandleInfoKHR-handle-01462", info->fence,
                         error_obj.location.dot(Field::pImportFenceWin32HandleInfo), "both handle (%p) and name (%p) are non-NULL",
                         info->handle, reinterpret_cast<const void *>(info->name));
    }
    return skip;
}

bool Device::manual_PreCallValidateGetFenceWin32HandleKHR(VkDevice device, const VkFenceGetWin32HandleInfoKHR *info,
                                                          HANDLE *pHandle, const Context &context) const {
    const auto &error_obj = context.error_obj;
    return ValidateExternalFenceHandleType(info->fence, "VUID-VkFenceGetWin32HandleInfoKHR-handleType-01452",
                                           error_obj.location.dot(Field::pGetWin32HandleInfo).dot(Field::handleType),
                                           info->handleType, kFenceWin32HandleTypes);
}
#endif

#ifdef VK_USE_PLATFORM_METAL_EXT
bool Device::ExportMetalObjectsPNextUtil(VkExportMetalObjectTypeFlagBitsEXT bit, const char *vuid, const Location &loc,
                                         const char *sType, const void *pNext) const {
    bool skip = false;
    auto export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(pNext);
    while (export_metal_object_info) {
        if (export_metal_object_info->exportObjectType != bit) {
            skip |= LogError(vuid, device, loc,
                             "The pNext chain contains a VkExportMetalObjectCreateInfoEXT whose exportObjectType = %s, but only "
                             "VkExportMetalObjectCreateInfoEXT structs with exportObjectType of %s are allowed.",
                             string_VkExportMetalObjectTypeFlagBitsEXT(export_metal_object_info->exportObjectType), sType);
        }
        export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(export_metal_object_info->pNext);
    }
    return skip;
}

#endif  // VK_USE_PLATFORM_METAL_EXT

namespace {

// Uses bool where the pointer is not needed to remove the ifdef macros from the core logic
struct ExternalOperationsInfo {
    bool import_info_win32 = false;
    bool import_info_win32_nv = false;
    bool export_info_win32 = false;
    bool export_info_win32_nv = false;

    const VkImportMemoryFdInfoKHR *import_info_fd = nullptr;
    const VkImportMemoryHostPointerInfoEXT *import_info_host_pointer = nullptr;

    const VkExportMemoryAllocateInfo *export_info = nullptr;
    const VkExportMemoryAllocateInfoNV *export_info_nv = nullptr;

    uint32_t total_import_ops = 0;
    bool has_export = false;
};

// vkspec.html#memory-import-operation describes all the various ways for import operations
ExternalOperationsInfo GetExternalOperationsInfo(const void *pNext) {
    ExternalOperationsInfo ext = {};

#ifdef VK_USE_PLATFORM_WIN32_KHR
    // VK_KHR_external_memory_win32
    auto import_info_win32 = vku::FindStructInPNextChain<VkImportMemoryWin32HandleInfoKHR>(pNext);
    ext.import_info_win32 = (import_info_win32 && import_info_win32->handleType);
    ext.total_import_ops += ext.import_info_win32;

    auto import_info_win32_nv = vku::FindStructInPNextChain<VkImportMemoryWin32HandleInfoNV>(pNext);
    ext.import_info_win32_nv = (import_info_win32_nv && import_info_win32_nv->handleType);
    ext.total_import_ops += ext.import_info_win32_nv;

    ext.export_info_win32 = vku::FindStructInPNextChain<VkExportMemoryWin32HandleInfoKHR>(pNext) != nullptr;

    ext.export_info_win32_nv = vku::FindStructInPNextChain<VkExportMemoryWin32HandleInfoNV>(pNext) != nullptr;
#endif

    // VK_KHR_external_memory_fd
    ext.import_info_fd = vku::FindStructInPNextChain<VkImportMemoryFdInfoKHR>(pNext);
    ext.total_import_ops += (ext.import_info_fd && ext.import_info_fd->handleType);

    // VK_EXT_external_memory_host
    ext.import_info_host_pointer = vku::FindStructInPNextChain<VkImportMemoryHostPointerInfoEXT>(pNext);
    ext.total_import_ops += (ext.import_info_host_pointer && ext.import_info_host_pointer->handleType);

    // All exports need a VkExportMemoryAllocateInfo or they are ignored
    // VK_KHR_external_memory
    ext.export_info = vku::FindStructInPNextChain<VkExportMemoryAllocateInfo>(pNext);
    ext.has_export |= (ext.export_info && ext.export_info->handleTypes);

    // VK_NV_external_memory
    ext.export_info_nv = vku::FindStructInPNextChain<VkExportMemoryAllocateInfoNV>(pNext);
    ext.has_export |= (ext.export_info_nv && ext.export_info_nv->handleTypes);

#ifdef VK_USE_PLATFORM_ANDROID_KHR
    // VK_ANDROID_external_memory_android_hardware_buffer
    auto import_info_ahb = vku::FindStructInPNextChain<VkImportAndroidHardwareBufferInfoANDROID>(pNext);
    ext.total_import_ops += (import_info_ahb && import_info_ahb->buffer);
#endif

#ifdef VK_USE_PLATFORM_FUCHSIA
    // VK_FUCHSIA_external_memory
    auto import_info_zircon = vku::FindStructInPNextChain<VkImportMemoryZirconHandleInfoFUCHSIA>(pNext);
    ext.total_import_ops += (import_info_zircon && import_info_zircon->handleType);

    // VK_FUCHSIA_buffer_collection
    // NOTE: There's no handleType on VkImportMemoryBufferCollectionFUCHSIA, so we can't check that, and from the "Valid Usage
    // (Implicit)" collection has to  always be valid.
    ext.total_import_ops += vku::FindStructInPNextChain<VkImportMemoryBufferCollectionFUCHSIA>(pNext) != nullptr;
#endif

#ifdef VK_USE_PLATFORM_SCREEN_QNX
    // VK_QNX_external_memory_screen_buffer
    auto import_info_qnx = vku::FindStructInPNextChain<VkImportScreenBufferInfoQNX>(pNext);
    ext.total_import_ops += (import_info_qnx && import_info_qnx->buffer);
#endif  // VK_USE_PLATFORM_SCREEN_QNX

#ifdef VK_USE_PLATFORM_METAL_EXT
    // VK_EXT_external_memory_metal
    auto import_info_metal = vku::FindStructInPNextChain<VkImportMemoryMetalHandleInfoEXT>(pNext);
    ext.total_import_ops += (import_info_metal && import_info_metal->handle);
#endif // VK_USE_PLATFORM_METAL_EXT

    return ext;
}
}  // namespace

bool Device::ValidateAllocateMemoryExternal(VkDevice device, const VkMemoryAllocateInfo &allocate_info, VkMemoryAllocateFlags flags,
                                            const Location &allocate_info_loc) const {
    bool skip = false;

    // Used to remove platform ifdef logic below
    const ExternalOperationsInfo ext = GetExternalOperationsInfo(allocate_info.pNext);

    if (!ext.has_export && ext.total_import_ops == 0 && allocate_info.allocationSize == 0) {
        skip |= LogError("VUID-VkMemoryAllocateInfo-allocationSize-07897", device, allocate_info_loc.dot(Field::allocationSize),
                         "is 0.");
    }

    auto opaque_alloc_info = vku::FindStructInPNextChain<VkMemoryOpaqueCaptureAddressAllocateInfo>(allocate_info.pNext);
    if (opaque_alloc_info && opaque_alloc_info->opaqueCaptureAddress != 0) {
        const Location address_loc =
            allocate_info_loc.pNext(Struct::VkMemoryOpaqueCaptureAddressAllocateInfo, Field::opaqueCaptureAddress);
        if (!(flags & VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT)) {
            skip |= LogError("VUID-VkMemoryAllocateInfo-opaqueCaptureAddress-03329", device, address_loc,
                             "is non-zero (%" PRIu64
                             ") so VkMemoryAllocateFlagsInfo::flags must include "
                             "VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_CAPTURE_REPLAY_BIT.",
                             opaque_alloc_info->opaqueCaptureAddress);
        }

        if (ext.import_info_host_pointer) {
            skip |= LogError("VUID-VkMemoryAllocateInfo-pNext-03332", device, address_loc,
                             "is non-zero (%" PRIu64 ") but the pNext chain includes a VkImportMemoryHostPointerInfoEXT structure.",
                             opaque_alloc_info->opaqueCaptureAddress);
        }

        if (ext.total_import_ops > 0) {
            skip |=
                LogError("VUID-VkMemoryAllocateInfo-opaqueCaptureAddress-03333", device, address_loc,
                         "is non-zero (%" PRIu64 ") but an import operation is defined.", opaque_alloc_info->opaqueCaptureAddress);
        }
    }

    if (ext.total_import_ops > 1) {
        skip |= LogError("VUID-VkMemoryAllocateInfo-None-06657", device, allocate_info_loc,
                         "%" PRIu32 " import operations are defined", ext.total_import_ops);
    }

    if (ext.export_info) {
        if (ext.export_info_nv) {
            skip |= LogError("VUID-VkMemoryAllocateInfo-pNext-00640", device, allocate_info_loc,
                             "pNext chain includes both VkExportMemoryAllocateInfo and "
                             "VkExportMemoryAllocateInfoNV");
        }
        if (ext.export_info_win32_nv) {
            skip |= LogError("VUID-VkMemoryAllocateInfo-pNext-00640", device, allocate_info_loc,
                             "pNext chain includes both VkExportMemoryAllocateInfo and "
                             "VkExportMemoryWin32HandleInfoNV");
        }
    }

    if (ext.import_info_win32 && ext.import_info_win32_nv) {
        skip |= LogError("VUID-VkMemoryAllocateInfo-pNext-00641", device, allocate_info_loc,
                         "pNext chain includes both VkImportMemoryWin32HandleInfoKHR and "
                         "VkImportMemoryWin32HandleInfoNV");
    }

    if (ext.import_info_fd && ext.import_info_fd->handleType != 0) {
        if (ext.import_info_fd->fd < 0) {
            skip |= LogError("VUID-VkImportMemoryFdInfoKHR-handleType-00670", device,
                             allocate_info_loc.pNext(Struct::VkImportMemoryFdInfoKHR, Field::fd),
                             "(%d) is not a valid POSIX file descriptor.", ext.import_info_fd->fd);
        }
        if (ext.import_info_fd->handleType != VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT &&
            ext.import_info_fd->handleType != VK_EXTERNAL_MEMORY_HANDLE_TYPE_DMA_BUF_BIT_EXT) {
            skip |= LogError("VUID-VkImportMemoryFdInfoKHR-handleType-00669", device,
                             allocate_info_loc.pNext(Struct::VkImportMemoryFdInfoKHR, Field::handleType), "%s is not allowed.",
                             string_VkExternalMemoryHandleTypeFlagBits(ext.import_info_fd->handleType));
        }
    }

    if (ext.import_info_host_pointer && ext.import_info_host_pointer->handleType != 0) {
        if (ext.import_info_host_pointer->handleType != VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT &&
            ext.import_info_host_pointer->handleType != VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_MAPPED_FOREIGN_MEMORY_BIT_EXT) {
            skip |= LogError("VUID-VkImportMemoryHostPointerInfoEXT-handleType-01748", device,
                             allocate_info_loc.pNext(Struct::VkImportMemoryHostPointerInfoEXT, Field::handleType), "is %s.",
                             string_VkExternalMemoryHandleTypeFlagBits(ext.import_info_host_pointer->handleType));
        }

        const VkDeviceSize host_pointer = reinterpret_cast<VkDeviceSize>(ext.import_info_host_pointer->pHostPointer);
        if (SafeModulo(host_pointer, phys_dev_ext_props.external_memory_host_props.minImportedHostPointerAlignment) != 0) {
            skip |= LogError("VUID-VkImportMemoryHostPointerInfoEXT-pHostPointer-01749", device,
                             allocate_info_loc.pNext(Struct::VkImportMemoryHostPointerInfoEXT, Field::pHostPointer),
                             "(0x%" PRIxLEAST64
                             ") is not aligned "
                             "to minImportedHostPointerAlignment (%" PRIuLEAST64 ")",
                             host_pointer, phys_dev_ext_props.external_memory_host_props.minImportedHostPointerAlignment);
        }

        if (SafeModulo(allocate_info.allocationSize,
                       phys_dev_ext_props.external_memory_host_props.minImportedHostPointerAlignment) != 0) {
            skip |= LogError("VUID-VkMemoryAllocateInfo-allocationSize-01745", device, allocate_info_loc.dot(Field::allocationSize),
                             "(%" PRIuLEAST64 ") is not a multiple of minImportedHostPointerAlignment (%" PRIuLEAST64 ")",
                             allocate_info.allocationSize,
                             phys_dev_ext_props.external_memory_host_props.minImportedHostPointerAlignment);
        }

        // only dispatch if known valid handle and host pointer
        if (!skip) {
            VkMemoryHostPointerPropertiesEXT host_pointer_props = vku::InitStructHelper();
            DispatchGetMemoryHostPointerPropertiesEXT(device, ext.import_info_host_pointer->handleType,
                                                      ext.import_info_host_pointer->pHostPointer, &host_pointer_props);
            if (((1 << allocate_info.memoryTypeIndex) & host_pointer_props.memoryTypeBits) == 0) {
                skip |= LogError(
                    "VUID-VkMemoryAllocateInfo-memoryTypeIndex-01744", device, allocate_info_loc.dot(Field::memoryTypeIndex),
                    "is %" PRIu32 " but VkMemoryHostPointerPropertiesEXT::memoryTypeBits is 0x%" PRIx32 " with handleType %s.",
                    allocate_info.memoryTypeIndex, host_pointer_props.memoryTypeBits,
                    string_VkExternalMemoryHandleTypeFlagBits(ext.import_info_host_pointer->handleType));
            }
        }

        auto dedicated_allocate_info = vku::FindStructInPNextChain<VkMemoryDedicatedAllocateInfo>(allocate_info.pNext);
        if (dedicated_allocate_info) {
            if (dedicated_allocate_info->buffer != VK_NULL_HANDLE) {
                skip |= LogError("VUID-VkMemoryAllocateInfo-pNext-02806", device,
                                 allocate_info_loc.pNext(Struct::VkMemoryDedicatedAllocateInfo, Field::buffer),
                                 "is %s but also using a host import with VkImportMemoryHostPointerInfoEXT.",
                                 FormatHandle(dedicated_allocate_info->buffer).c_str());
            } else if (dedicated_allocate_info->image != VK_NULL_HANDLE) {
                skip |= LogError("VUID-VkMemoryAllocateInfo-pNext-02806", device,
                                 allocate_info_loc.pNext(Struct::VkMemoryDedicatedAllocateInfo, Field::image),
                                 "is %s but also using a host import with VkImportMemoryHostPointerInfoEXT.",
                                 FormatHandle(dedicated_allocate_info->image).c_str());
            }
        }
    }

#ifdef VK_USE_PLATFORM_METAL_EXT
    skip |= ExportMetalObjectsPNextUtil(VK_EXPORT_METAL_OBJECT_TYPE_METAL_BUFFER_BIT_EXT, "VUID-VkMemoryAllocateInfo-pNext-06780",
                                        allocate_info_loc, "VK_EXPORT_METAL_OBJECT_TYPE_METAL_BUFFER_BIT_EXT", allocate_info.pNext);
#endif  // VK_USE_PLATFORM_METAL_EXT

    return skip;
}
}  // namespace stateless
