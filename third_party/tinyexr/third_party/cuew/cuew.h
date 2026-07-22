/*
 * Copyright 2011-2014 Blender Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License
 */

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#pragma clang diagnostic ignored "-Wpadded"
#if __has_warning("-Wdocumentation-deprecated-sync")
  #pragma clang diagnostic ignored "-Wdocumentation-deprecated-sync"
#endif
#if __has_warning("-Wreserved-identifier")
  #pragma clang diagnostic ignored "-Wreserved-identifier"
#endif
#endif

#ifndef __CUEW_H__
#define __CUEW_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

/* Defines. */
#define CUEW_VERSION_MAJOR 2
#define CUEW_VERSION_MINOR 0

#define cuCtxCreate_v3 cuCtxCreate_v3
#define cuTexRefSetAddress2D cuTexRefSetAddress2D_v3
#define cuGraphInstantiate cuGraphInstantiateWithFlags
#define CUDA_VERSION 12010
#define CU_UUID_HAS_BEEN_DEFINED
#define CU_IPC_HANDLE_SIZE 64
#define CU_STREAM_LEGACY ((CUstream)0x1)
#define CU_STREAM_PER_THREAD ((CUstream)0x2)
#define CU_COMPUTE_ACCELERATED_TARGET_BASE 0x10000
#define CU_KERNEL_NODE_ATTRIBUTE_ACCESS_POLICY_WINDOW CU_LAUNCH_ATTRIBUTE_ACCESS_POLICY_WINDOW
#define CU_KERNEL_NODE_ATTRIBUTE_COOPERATIVE CU_LAUNCH_ATTRIBUTE_COOPERATIVE
#define CU_KERNEL_NODE_ATTRIBUTE_CLUSTER_DIMENSION CU_LAUNCH_ATTRIBUTE_CLUSTER_DIMENSION
#define CU_KERNEL_NODE_ATTRIBUTE_CLUSTER_SCHEDULING_POLICY_PREFERENCE CU_LAUNCH_ATTRIBUTE_CLUSTER_SCHEDULING_POLICY_PREFERENCE
#define CU_KERNEL_NODE_ATTRIBUTE_PRIORITY CU_LAUNCH_ATTRIBUTE_PRIORITY
#define CU_KERNEL_NODE_ATTRIBUTE_MEM_SYNC_DOMAIN_MAP CU_LAUNCH_ATTRIBUTE_MEM_SYNC_DOMAIN_MAP
#define CU_KERNEL_NODE_ATTRIBUTE_MEM_SYNC_DOMAIN CU_LAUNCH_ATTRIBUTE_MEM_SYNC_DOMAIN
#define CU_STREAM_ATTRIBUTE_ACCESS_POLICY_WINDOW CU_LAUNCH_ATTRIBUTE_ACCESS_POLICY_WINDOW
#define CU_STREAM_ATTRIBUTE_SYNCHRONIZATION_POLICY CU_LAUNCH_ATTRIBUTE_SYNCHRONIZATION_POLICY
#define CU_STREAM_ATTRIBUTE_PRIORITY CU_LAUNCH_ATTRIBUTE_PRIORITY
#define CU_STREAM_ATTRIBUTE_MEM_SYNC_DOMAIN_MAP CU_LAUNCH_ATTRIBUTE_MEM_SYNC_DOMAIN_MAP
#define CU_STREAM_ATTRIBUTE_MEM_SYNC_DOMAIN CU_LAUNCH_ATTRIBUTE_MEM_SYNC_DOMAIN
#define CU_MEMHOSTALLOC_PORTABLE 0x01
#define CU_MEMHOSTALLOC_DEVICEMAP 0x02
#define CU_MEMHOSTALLOC_WRITECOMBINED 0x04
#define CU_MEMHOSTREGISTER_PORTABLE 0x01
#define CU_MEMHOSTREGISTER_DEVICEMAP 0x02
#define CU_MEMHOSTREGISTER_IOMEMORY 0x04
#define CU_MEMHOSTREGISTER_READ_ONLY 0x08
#define CU_ARRAY_SPARSE_PROPERTIES_SINGLE_MIPTAIL 0x1
#define CU_TENSOR_MAP_NUM_QWORDS 16
#define CUDA_EXTERNAL_MEMORY_DEDICATED 0x1
#define CUDA_EXTERNAL_SEMAPHORE_SIGNAL_SKIP_NVSCIBUF_MEMSYNC 0x01
#define CUDA_EXTERNAL_SEMAPHORE_WAIT_SKIP_NVSCIBUF_MEMSYNC 0x02
#define CUDA_NVSCISYNC_ATTR_SIGNAL 0x1
#define CUDA_NVSCISYNC_ATTR_WAIT 0x2
#define CU_MEM_CREATE_USAGE_TILE_POOL 0x1
#define CUDA_COOPERATIVE_LAUNCH_MULTI_DEVICE_NO_PRE_LAUNCH_SYNC 0x01
#define CUDA_COOPERATIVE_LAUNCH_MULTI_DEVICE_NO_POST_LAUNCH_SYNC 0x02
#define CUDA_ARRAY3D_LAYERED 0x01
#define CUDA_ARRAY3D_2DARRAY 0x01
#define CUDA_ARRAY3D_SURFACE_LDST 0x02
#define CUDA_ARRAY3D_CUBEMAP 0x04
#define CUDA_ARRAY3D_TEXTURE_GATHER 0x08
#define CUDA_ARRAY3D_DEPTH_TEXTURE 0x10
#define CUDA_ARRAY3D_COLOR_ATTACHMENT 0x20
#define CUDA_ARRAY3D_SPARSE 0x40
#define CUDA_ARRAY3D_DEFERRED_MAPPING 0x80
#define CU_TRSA_OVERRIDE_FORMAT 0x01
#define CU_TRSF_READ_AS_INTEGER 0x01
#define CU_TRSF_NORMALIZED_COORDINATES 0x02
#define CU_TRSF_SRGB 0x10
#define CU_TRSF_DISABLE_TRILINEAR_OPTIMIZATION 0x20
#define CU_TRSF_SEAMLESS_CUBEMAP 0x40
#define CU_LAUNCH_PARAM_END_AS_INT 0x00
#define CU_LAUNCH_PARAM_END ((void*)CU_LAUNCH_PARAM_END_AS_INT)
#define CU_LAUNCH_PARAM_BUFFER_POINTER_AS_INT 0x01
#define CU_LAUNCH_PARAM_BUFFER_POINTER ((void*)CU_LAUNCH_PARAM_BUFFER_POINTER_AS_INT)
#define CU_LAUNCH_PARAM_BUFFER_SIZE_AS_INT 0x02
#define CU_LAUNCH_PARAM_BUFFER_SIZE ((void*)CU_LAUNCH_PARAM_BUFFER_SIZE_AS_INT)
#define CU_PARAM_TR_DEFAULT -1
#define CU_DEVICE_CPU ((CUdevice)-1)
#define CU_DEVICE_INVALID ((CUdevice)-2)

/* Functions which changed 3.1 -> 3.2 for 64 bit stuff,
 * the cuda library has both the old ones for compatibility and new
 * ones with _v2 postfix,
 */
#define cuDeviceTotalMem cuDeviceTotalMem_v2
#define cuCtxCreate cuCtxCreate_v2
#define cuModuleGetGlobal cuModuleGetGlobal_v2
#define cuMemGetInfo cuMemGetInfo_v2
#define cuMemAlloc cuMemAlloc_v2
#define cuMemAllocPitch cuMemAllocPitch_v2
#define cuMemFree cuMemFree_v2
#define cuMemGetAddressRange cuMemGetAddressRange_v2
#define cuMemAllocHost cuMemAllocHost_v2
#define cuMemHostGetDevicePointer cuMemHostGetDevicePointer_v2
#define cuMemcpyHtoD cuMemcpyHtoD_v2
#define cuMemcpyDtoH cuMemcpyDtoH_v2
#define cuMemcpyDtoD cuMemcpyDtoD_v2
#define cuMemcpyDtoA cuMemcpyDtoA_v2
#define cuMemcpyAtoD cuMemcpyAtoD_v2
#define cuMemcpyHtoA cuMemcpyHtoA_v2
#define cuMemcpyAtoH cuMemcpyAtoH_v2
#define cuMemcpyAtoA cuMemcpyAtoA_v2
#define cuMemcpyHtoAAsync cuMemcpyHtoAAsync_v2
#define cuMemcpyAtoHAsync cuMemcpyAtoHAsync_v2
#define cuMemcpy2D cuMemcpy2D_v2
#define cuMemcpy2DUnaligned cuMemcpy2DUnaligned_v2
#define cuMemcpy3D cuMemcpy3D_v2
#define cuMemcpyHtoDAsync cuMemcpyHtoDAsync_v2
#define cuMemcpyDtoHAsync cuMemcpyDtoHAsync_v2
#define cuMemcpyDtoDAsync cuMemcpyDtoDAsync_v2
#define cuMemcpy2DAsync cuMemcpy2DAsync_v2
#define cuMemcpy3DAsync cuMemcpy3DAsync_v2
#define cuMemsetD8 cuMemsetD8_v2
#define cuMemsetD16 cuMemsetD16_v2
#define cuMemsetD32 cuMemsetD32_v2
#define cuMemsetD2D8 cuMemsetD2D8_v2
#define cuMemsetD2D16 cuMemsetD2D16_v2
#define cuMemsetD2D32 cuMemsetD2D32_v2
#define cuArrayCreate cuArrayCreate_v2
#define cuArrayGetDescriptor cuArrayGetDescriptor_v2
#define cuArray3DCreate cuArray3DCreate_v2
#define cuArray3DGetDescriptor cuArray3DGetDescriptor_v2
#define cuTexRefSetAddress cuTexRefSetAddress_v2
#define cuTexRefGetAddress cuTexRefGetAddress_v2
#define cuGraphicsResourceGetMappedPointer cuGraphicsResourceGetMappedPointer_v2
#define cuCtxDestroy cuCtxDestroy_v2
#define cuCtxPopCurrent cuCtxPopCurrent_v2
#define cuCtxPushCurrent cuCtxPushCurrent_v2
#define cuStreamDestroy cuStreamDestroy_v2
#define cuEventDestroy cuEventDestroy_v2
#define cuLinkCreate cuLinkCreate_v2
#define cuLinkAddData cuLinkAddData_v2
#define cuLinkAddFile cuLinkAddFile_v2
#define cuMemHostRegister cuMemHostRegister_v2
#define cuGraphicsResourceSetMapFlags cuGraphicsResourceSetMapFlags_v2
#define cuStreamBeginCapture cuStreamBeginCapture_v2
#define cuDevicePrimaryCtxRelease cuDevicePrimaryCtxRelease_v2
#define cuDevicePrimaryCtxReset cuDevicePrimaryCtxReset_v2
#define cuDevicePrimaryCtxSetFlags cuDevicePrimaryCtxSetFlags_v2
#define cuDeviceGetUuid_v2 cuDeviceGetUuid_v2
#define cuIpcOpenMemHandle cuIpcOpenMemHandle_v2
#define cuGraphExecUpdate cuGraphExecUpdate_v2
#define cuGetProcAddress cuGetProcAddress_v2
#define cuGraphAddKernelNode cuGraphAddKernelNode_v2
#define cuGraphKernelNodeGetParams cuGraphKernelNodeGetParams_v2
#define cuGraphKernelNodeSetParams cuGraphKernelNodeSetParams_v2
#define cuGraphExecKernelNodeSetParams cuGraphExecKernelNodeSetParams_v2
#define cuStreamWriteValue32 cuStreamWriteValue32_v2
#define cuStreamWaitValue32 cuStreamWaitValue32_v2
#define cuStreamWriteValue64 cuStreamWriteValue64_v2
#define cuStreamWaitValue64 cuStreamWaitValue64_v2
#define cuStreamBatchMemOp cuStreamBatchMemOp_v2
#define cuStreamGetCaptureInfo cuStreamGetCaptureInfo_v2
#define cuStreamGetCaptureInfo_v2 cuStreamGetCaptureInfo_v2
#define cuGLCtxCreate cuGLCtxCreate_v2
#define cuGLMapBufferObject cuGLMapBufferObject_v2
#define cuGLMapBufferObjectAsync cuGLMapBufferObjectAsync_v2
#define cuGLGetDevices cuGLGetDevices_v2

/* Types. */
#ifdef _MSC_VER
typedef unsigned __int32 cuuint32_t;
typedef unsigned __int64 cuuint64_t;
/* Assume VS2017 or later */
#include <stdint.h>
#else
#include <stdint.h>
typedef uint32_t cuuint32_t;
typedef uint64_t cuuint64_t;
#endif

#if defined(__x86_64) || defined(AMD64) || defined(_M_AMD64) || defined (__aarch64__)
typedef unsigned long long CUdeviceptr;
#else
typedef unsigned int CUdeviceptr;
#endif

#ifdef _WIN32
#  define CUDAAPI __stdcall
#  define CUDA_CB __stdcall
#else
#  define CUDAAPI
#  define CUDA_CB
#endif

#if !defined(__CUDACC__)
#  define __device_builtin__
#else
#  define __device_builtin__ __location__(device_builtin)
#endif

typedef __device_builtin__ struct CUstream_st *cudaStream_t;

typedef unsigned long long CUdeviceptr_v2;
typedef int CUdevice_v1;
typedef CUdevice_v1 CUdevice;
typedef struct CUctx_st* CUcontext;
typedef struct CUmod_st* CUmodule;
typedef struct CUfunc_st* CUfunction;
typedef struct CUlib_st* CUlibrary;
typedef struct CUkern_st* CUkernel;
typedef struct CUarray_st* CUarray;
typedef struct CUmipmappedArray_st* CUmipmappedArray;
typedef struct CUtexref_st* CUtexref;
typedef struct CUsurfref_st* CUsurfref;
typedef struct CUevent_st* CUevent;
typedef struct CUstream_st* CUstream;
typedef struct CUgraphicsResource_st* CUgraphicsResource;
typedef unsigned long long CUtexObject_v1;
typedef CUtexObject_v1 CUtexObject;
typedef unsigned long long CUsurfObject_v1;
typedef CUsurfObject_v1 CUsurfObject;
typedef struct CUextMemory_st* CUexternalMemory;
typedef struct CUextSemaphore_st* CUexternalSemaphore;
typedef struct CUgraph_st* CUgraph;
typedef struct CUgraphNode_st* CUgraphNode;
typedef struct CUgraphExec_st* CUgraphExec;
typedef struct CUmemPoolHandle_st* CUmemoryPool;
typedef struct CUuserObject_st* CUuserObject;

typedef struct CUuuid_st {
  char bytes[16];
} CUuuid;

typedef struct CUipcEventHandle_st {
  char reserved[CU_IPC_HANDLE_SIZE];
} CUipcEventHandle_v1;

typedef CUipcEventHandle_v1 CUipcEventHandle;

typedef struct CUipcMemHandle_st {
  char reserved[CU_IPC_HANDLE_SIZE];
} CUipcMemHandle_v1;

typedef CUipcMemHandle_v1 CUipcMemHandle;

typedef enum CUipcMem_flags_enum {
  CU_IPC_MEM_LAZY_ENABLE_PEER_ACCESS = 0x1,
} CUipcMem_flags;

typedef enum CUmemAttach_flags_enum {
  CU_MEM_ATTACH_GLOBAL = 0x1,
  CU_MEM_ATTACH_HOST = 0x2,
  CU_MEM_ATTACH_SINGLE = 0x4,
} CUmemAttach_flags;

typedef enum CUctx_flags_enum {
  CU_CTX_SCHED_AUTO = 0x00,
  CU_CTX_SCHED_SPIN = 0x01,
  CU_CTX_SCHED_YIELD = 0x02,
  CU_CTX_SCHED_BLOCKING_SYNC = 0x04,
  CU_CTX_BLOCKING_SYNC = 0x04,
  CU_CTX_SCHED_MASK = 0x07,
  CU_CTX_MAP_HOST = 0x08,
  CU_CTX_LMEM_RESIZE_TO_MAX = 0x10,
  CU_CTX_COREDUMP_ENABLE = 0x20,
  CU_CTX_USER_COREDUMP_ENABLE = 0x40,
  CU_CTX_SYNC_MEMOPS = 0x80,
  CU_CTX_FLAGS_MASK = 0xFF,
} CUctx_flags;

typedef enum CUevent_sched_flags_enum {
  CU_EVENT_SCHED_AUTO = 0x00,
  CU_EVENT_SCHED_SPIN = 0x01,
  CU_EVENT_SCHED_YIELD = 0x02,
  CU_EVENT_SCHED_BLOCKING_SYNC = 0x04,
} CUevent_sched_flags;

typedef enum cl_event_flags_enum {
  NVCL_EVENT_SCHED_AUTO = 0x00,
  NVCL_EVENT_SCHED_SPIN = 0x01,
  NVCL_EVENT_SCHED_YIELD = 0x02,
  NVCL_EVENT_SCHED_BLOCKING_SYNC = 0x04,
} cl_event_flags;

typedef enum cl_context_flags_enum {
  NVCL_CTX_SCHED_AUTO = 0x00,
  NVCL_CTX_SCHED_SPIN = 0x01,
  NVCL_CTX_SCHED_YIELD = 0x02,
  NVCL_CTX_SCHED_BLOCKING_SYNC = 0x04,
} cl_context_flags;

typedef enum CUstream_flags_enum {
  CU_STREAM_DEFAULT = 0x0,
  CU_STREAM_NON_BLOCKING = 0x1,
} CUstream_flags;

typedef enum CUevent_flags_enum {
  CU_EVENT_DEFAULT = 0x0,
  CU_EVENT_BLOCKING_SYNC = 0x1,
  CU_EVENT_DISABLE_TIMING = 0x2,
  CU_EVENT_INTERPROCESS = 0x4,
} CUevent_flags;

typedef enum CUevent_record_flags_enum {
  CU_EVENT_RECORD_DEFAULT = 0x0,
  CU_EVENT_RECORD_EXTERNAL = 0x1,
} CUevent_record_flags;

typedef enum CUevent_wait_flags_enum {
  CU_EVENT_WAIT_DEFAULT = 0x0,
  CU_EVENT_WAIT_EXTERNAL = 0x1,
} CUevent_wait_flags;

typedef enum CUstreamWaitValue_flags_enum {
  CU_STREAM_WAIT_VALUE_GEQ = 0x0,
  CU_STREAM_WAIT_VALUE_EQ = 0x1,
  CU_STREAM_WAIT_VALUE_AND = 0x2,
  CU_STREAM_WAIT_VALUE_NOR = 0x3,
  CU_STREAM_WAIT_VALUE_FLUSH = (1 << 30),
} CUstreamWaitValue_flags;

typedef enum CUstreamWriteValue_flags_enum {
  CU_STREAM_WRITE_VALUE_DEFAULT = 0x0,
  CU_STREAM_WRITE_VALUE_NO_MEMORY_BARRIER = 0x1,
} CUstreamWriteValue_flags;

typedef enum CUstreamBatchMemOpType_enum {
  CU_STREAM_MEM_OP_WAIT_VALUE_32 = 1,
  CU_STREAM_MEM_OP_WRITE_VALUE_32 = 2,
  CU_STREAM_MEM_OP_WAIT_VALUE_64 = 4,
  CU_STREAM_MEM_OP_WRITE_VALUE_64 = 5,
  CU_STREAM_MEM_OP_BARRIER = 6,
  CU_STREAM_MEM_OP_FLUSH_REMOTE_WRITES = 3,
} CUstreamBatchMemOpType;

typedef enum CUstreamMemoryBarrier_flags_enum {
  CU_STREAM_MEMORY_BARRIER_TYPE_SYS = 0x0,
  CU_STREAM_MEMORY_BARRIER_TYPE_GPU = 0x1,
} CUstreamMemoryBarrier_flags;

typedef union CUstreamBatchMemOpParams_union {
  CUstreamBatchMemOpType operation;
  struct CUstreamMemOpWaitValueParams_st {
    CUstreamBatchMemOpType operation;
    CUdeviceptr address;
    union {
      cuuint32_t value;
      cuuint64_t value64;
    };
    unsigned int flags;
    CUdeviceptr alias;
  } waitValue;
  struct CUstreamMemOpWriteValueParams_st {
    CUstreamBatchMemOpType operation;
    CUdeviceptr address;
    union {
      cuuint32_t value;
      cuuint64_t value64;
    };
    unsigned int flags;
    CUdeviceptr alias;
  } writeValue;
  struct CUstreamMemOpFlushRemoteWritesParams_st {
    CUstreamBatchMemOpType operation;
    unsigned int flags;
  } flushRemoteWrites;
  struct CUstreamMemOpMemoryBarrierParams_st {
    CUstreamBatchMemOpType operation;
    unsigned int flags;
  } memoryBarrier;
  cuuint64_t pad[6];
} CUstreamBatchMemOpParams_v1;
typedef CUstreamBatchMemOpParams_v1 CUstreamBatchMemOpParams;

typedef struct CUDA_BATCH_MEM_OP_NODE_PARAMS_st {
  CUcontext ctx;
  unsigned int count;
  CUstreamBatchMemOpParams* paramArray;
  unsigned int flags;
} CUDA_BATCH_MEM_OP_NODE_PARAMS;

typedef enum CUoccupancy_flags_enum {
  CU_OCCUPANCY_DEFAULT = 0x0,
  CU_OCCUPANCY_DISABLE_CACHING_OVERRIDE = 0x1,
} CUoccupancy_flags;

typedef enum CUstreamUpdateCaptureDependencies_flags_enum {
  CU_STREAM_ADD_CAPTURE_DEPENDENCIES = 0x0,
  CU_STREAM_SET_CAPTURE_DEPENDENCIES = 0x1,
} CUstreamUpdateCaptureDependencies_flags;

typedef enum CUarray_format_enum {
  CU_AD_FORMAT_UNSIGNED_INT8 = 0x01,
  CU_AD_FORMAT_UNSIGNED_INT16 = 0x02,
  CU_AD_FORMAT_UNSIGNED_INT32 = 0x03,
  CU_AD_FORMAT_SIGNED_INT8 = 0x08,
  CU_AD_FORMAT_SIGNED_INT16 = 0x09,
  CU_AD_FORMAT_SIGNED_INT32 = 0x0a,
  CU_AD_FORMAT_HALF = 0x10,
  CU_AD_FORMAT_FLOAT = 0x20,
  CU_AD_FORMAT_NV12 = 0xb0,
  CU_AD_FORMAT_UNORM_INT8X1 = 0xc0,
  CU_AD_FORMAT_UNORM_INT8X2 = 0xc1,
  CU_AD_FORMAT_UNORM_INT8X4 = 0xc2,
  CU_AD_FORMAT_UNORM_INT16X1 = 0xc3,
  CU_AD_FORMAT_UNORM_INT16X2 = 0xc4,
  CU_AD_FORMAT_UNORM_INT16X4 = 0xc5,
  CU_AD_FORMAT_SNORM_INT8X1 = 0xc6,
  CU_AD_FORMAT_SNORM_INT8X2 = 0xc7,
  CU_AD_FORMAT_SNORM_INT8X4 = 0xc8,
  CU_AD_FORMAT_SNORM_INT16X1 = 0xc9,
  CU_AD_FORMAT_SNORM_INT16X2 = 0xca,
  CU_AD_FORMAT_SNORM_INT16X4 = 0xcb,
  CU_AD_FORMAT_BC1_UNORM = 0x91,
  CU_AD_FORMAT_BC1_UNORM_SRGB = 0x92,
  CU_AD_FORMAT_BC2_UNORM = 0x93,
  CU_AD_FORMAT_BC2_UNORM_SRGB = 0x94,
  CU_AD_FORMAT_BC3_UNORM = 0x95,
  CU_AD_FORMAT_BC3_UNORM_SRGB = 0x96,
  CU_AD_FORMAT_BC4_UNORM = 0x97,
  CU_AD_FORMAT_BC4_SNORM = 0x98,
  CU_AD_FORMAT_BC5_UNORM = 0x99,
  CU_AD_FORMAT_BC5_SNORM = 0x9a,
  CU_AD_FORMAT_BC6H_UF16 = 0x9b,
  CU_AD_FORMAT_BC6H_SF16 = 0x9c,
  CU_AD_FORMAT_BC7_UNORM = 0x9d,
  CU_AD_FORMAT_BC7_UNORM_SRGB = 0x9e,
} CUarray_format;

typedef enum CUaddress_mode_enum {
  CU_TR_ADDRESS_MODE_WRAP = 0,
  CU_TR_ADDRESS_MODE_CLAMP = 1,
  CU_TR_ADDRESS_MODE_MIRROR = 2,
  CU_TR_ADDRESS_MODE_BORDER = 3,
} CUaddress_mode;

typedef enum CUfilter_mode_enum {
  CU_TR_FILTER_MODE_POINT = 0,
  CU_TR_FILTER_MODE_LINEAR = 1,
} CUfilter_mode;

typedef enum CUdevice_attribute_enum {
  CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_BLOCK = 1,
  CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_X = 2,
  CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Y = 3,
  CU_DEVICE_ATTRIBUTE_MAX_BLOCK_DIM_Z = 4,
  CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_X = 5,
  CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Y = 6,
  CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Z = 7,
  CU_DEVICE_ATTRIBUTE_MAX_SHARED_MEMORY_PER_BLOCK = 8,
  CU_DEVICE_ATTRIBUTE_SHARED_MEMORY_PER_BLOCK = 8,
  CU_DEVICE_ATTRIBUTE_TOTAL_CONSTANT_MEMORY = 9,
  CU_DEVICE_ATTRIBUTE_WARP_SIZE = 10,
  CU_DEVICE_ATTRIBUTE_MAX_PITCH = 11,
  CU_DEVICE_ATTRIBUTE_MAX_REGISTERS_PER_BLOCK = 12,
  CU_DEVICE_ATTRIBUTE_REGISTERS_PER_BLOCK = 12,
  CU_DEVICE_ATTRIBUTE_CLOCK_RATE = 13,
  CU_DEVICE_ATTRIBUTE_TEXTURE_ALIGNMENT = 14,
  CU_DEVICE_ATTRIBUTE_GPU_OVERLAP = 15,
  CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT = 16,
  CU_DEVICE_ATTRIBUTE_KERNEL_EXEC_TIMEOUT = 17,
  CU_DEVICE_ATTRIBUTE_INTEGRATED = 18,
  CU_DEVICE_ATTRIBUTE_CAN_MAP_HOST_MEMORY = 19,
  CU_DEVICE_ATTRIBUTE_COMPUTE_MODE = 20,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE1D_WIDTH = 21,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_WIDTH = 22,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_HEIGHT = 23,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_WIDTH = 24,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_HEIGHT = 25,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_DEPTH = 26,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LAYERED_WIDTH = 27,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LAYERED_HEIGHT = 28,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LAYERED_LAYERS = 29,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_ARRAY_WIDTH = 27,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_ARRAY_HEIGHT = 28,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_ARRAY_NUMSLICES = 29,
  CU_DEVICE_ATTRIBUTE_SURFACE_ALIGNMENT = 30,
  CU_DEVICE_ATTRIBUTE_CONCURRENT_KERNELS = 31,
  CU_DEVICE_ATTRIBUTE_ECC_ENABLED = 32,
  CU_DEVICE_ATTRIBUTE_PCI_BUS_ID = 33,
  CU_DEVICE_ATTRIBUTE_PCI_DEVICE_ID = 34,
  CU_DEVICE_ATTRIBUTE_TCC_DRIVER = 35,
  CU_DEVICE_ATTRIBUTE_MEMORY_CLOCK_RATE = 36,
  CU_DEVICE_ATTRIBUTE_GLOBAL_MEMORY_BUS_WIDTH = 37,
  CU_DEVICE_ATTRIBUTE_L2_CACHE_SIZE = 38,
  CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_MULTIPROCESSOR = 39,
  CU_DEVICE_ATTRIBUTE_ASYNC_ENGINE_COUNT = 40,
  CU_DEVICE_ATTRIBUTE_UNIFIED_ADDRESSING = 41,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE1D_LAYERED_WIDTH = 42,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE1D_LAYERED_LAYERS = 43,
  CU_DEVICE_ATTRIBUTE_CAN_TEX2D_GATHER = 44,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_GATHER_WIDTH = 45,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_GATHER_HEIGHT = 46,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_WIDTH_ALTERNATE = 47,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_HEIGHT_ALTERNATE = 48,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE3D_DEPTH_ALTERNATE = 49,
  CU_DEVICE_ATTRIBUTE_PCI_DOMAIN_ID = 50,
  CU_DEVICE_ATTRIBUTE_TEXTURE_PITCH_ALIGNMENT = 51,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURECUBEMAP_WIDTH = 52,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURECUBEMAP_LAYERED_WIDTH = 53,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURECUBEMAP_LAYERED_LAYERS = 54,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE1D_WIDTH = 55,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE2D_WIDTH = 56,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE2D_HEIGHT = 57,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE3D_WIDTH = 58,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE3D_HEIGHT = 59,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE3D_DEPTH = 60,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE1D_LAYERED_WIDTH = 61,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE1D_LAYERED_LAYERS = 62,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE2D_LAYERED_WIDTH = 63,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE2D_LAYERED_HEIGHT = 64,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACE2D_LAYERED_LAYERS = 65,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACECUBEMAP_WIDTH = 66,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACECUBEMAP_LAYERED_WIDTH = 67,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_SURFACECUBEMAP_LAYERED_LAYERS = 68,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE1D_LINEAR_WIDTH = 69,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LINEAR_WIDTH = 70,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LINEAR_HEIGHT = 71,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_LINEAR_PITCH = 72,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_MIPMAPPED_WIDTH = 73,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE2D_MIPMAPPED_HEIGHT = 74,
  CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR = 75,
  CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR = 76,
  CU_DEVICE_ATTRIBUTE_MAXIMUM_TEXTURE1D_MIPMAPPED_WIDTH = 77,
  CU_DEVICE_ATTRIBUTE_STREAM_PRIORITIES_SUPPORTED = 78,
  CU_DEVICE_ATTRIBUTE_GLOBAL_L1_CACHE_SUPPORTED = 79,
  CU_DEVICE_ATTRIBUTE_LOCAL_L1_CACHE_SUPPORTED = 80,
  CU_DEVICE_ATTRIBUTE_MAX_SHARED_MEMORY_PER_MULTIPROCESSOR = 81,
  CU_DEVICE_ATTRIBUTE_MAX_REGISTERS_PER_MULTIPROCESSOR = 82,
  CU_DEVICE_ATTRIBUTE_MANAGED_MEMORY = 83,
  CU_DEVICE_ATTRIBUTE_MULTI_GPU_BOARD = 84,
  CU_DEVICE_ATTRIBUTE_MULTI_GPU_BOARD_GROUP_ID = 85,
  CU_DEVICE_ATTRIBUTE_HOST_NATIVE_ATOMIC_SUPPORTED = 86,
  CU_DEVICE_ATTRIBUTE_SINGLE_TO_DOUBLE_PRECISION_PERF_RATIO = 87,
  CU_DEVICE_ATTRIBUTE_PAGEABLE_MEMORY_ACCESS = 88,
  CU_DEVICE_ATTRIBUTE_CONCURRENT_MANAGED_ACCESS = 89,
  CU_DEVICE_ATTRIBUTE_COMPUTE_PREEMPTION_SUPPORTED = 90,
  CU_DEVICE_ATTRIBUTE_CAN_USE_HOST_POINTER_FOR_REGISTERED_MEM = 91,
  CU_DEVICE_ATTRIBUTE_CAN_USE_STREAM_MEM_OPS_V1 = 92,
  CU_DEVICE_ATTRIBUTE_CAN_USE_64_BIT_STREAM_MEM_OPS_V1 = 93,
  CU_DEVICE_ATTRIBUTE_CAN_USE_STREAM_WAIT_VALUE_NOR_V1 = 94,
  CU_DEVICE_ATTRIBUTE_COOPERATIVE_LAUNCH = 95,
  CU_DEVICE_ATTRIBUTE_COOPERATIVE_MULTI_DEVICE_LAUNCH = 96,
  CU_DEVICE_ATTRIBUTE_MAX_SHARED_MEMORY_PER_BLOCK_OPTIN = 97,
  CU_DEVICE_ATTRIBUTE_CAN_FLUSH_REMOTE_WRITES = 98,
  CU_DEVICE_ATTRIBUTE_HOST_REGISTER_SUPPORTED = 99,
  CU_DEVICE_ATTRIBUTE_PAGEABLE_MEMORY_ACCESS_USES_HOST_PAGE_TABLES = 100,
  CU_DEVICE_ATTRIBUTE_DIRECT_MANAGED_MEM_ACCESS_FROM_HOST = 101,
  CU_DEVICE_ATTRIBUTE_VIRTUAL_ADDRESS_MANAGEMENT_SUPPORTED = 102,
  CU_DEVICE_ATTRIBUTE_VIRTUAL_MEMORY_MANAGEMENT_SUPPORTED = 102,
  CU_DEVICE_ATTRIBUTE_HANDLE_TYPE_POSIX_FILE_DESCRIPTOR_SUPPORTED = 103,
  CU_DEVICE_ATTRIBUTE_HANDLE_TYPE_WIN32_HANDLE_SUPPORTED = 104,
  CU_DEVICE_ATTRIBUTE_HANDLE_TYPE_WIN32_KMT_HANDLE_SUPPORTED = 105,
  CU_DEVICE_ATTRIBUTE_MAX_BLOCKS_PER_MULTIPROCESSOR = 106,
  CU_DEVICE_ATTRIBUTE_GENERIC_COMPRESSION_SUPPORTED = 107,
  CU_DEVICE_ATTRIBUTE_MAX_PERSISTING_L2_CACHE_SIZE = 108,
  CU_DEVICE_ATTRIBUTE_MAX_ACCESS_POLICY_WINDOW_SIZE = 109,
  CU_DEVICE_ATTRIBUTE_GPU_DIRECT_RDMA_WITH_CUDA_VMM_SUPPORTED = 110,
  CU_DEVICE_ATTRIBUTE_RESERVED_SHARED_MEMORY_PER_BLOCK = 111,
  CU_DEVICE_ATTRIBUTE_SPARSE_CUDA_ARRAY_SUPPORTED = 112,
  CU_DEVICE_ATTRIBUTE_READ_ONLY_HOST_REGISTER_SUPPORTED = 113,
  CU_DEVICE_ATTRIBUTE_TIMELINE_SEMAPHORE_INTEROP_SUPPORTED = 114,
  CU_DEVICE_ATTRIBUTE_MEMORY_POOLS_SUPPORTED = 115,
  CU_DEVICE_ATTRIBUTE_GPU_DIRECT_RDMA_SUPPORTED = 116,
  CU_DEVICE_ATTRIBUTE_GPU_DIRECT_RDMA_FLUSH_WRITES_OPTIONS = 117,
  CU_DEVICE_ATTRIBUTE_GPU_DIRECT_RDMA_WRITES_ORDERING = 118,
  CU_DEVICE_ATTRIBUTE_MEMPOOL_SUPPORTED_HANDLE_TYPES = 119,
  CU_DEVICE_ATTRIBUTE_CLUSTER_LAUNCH = 120,
  CU_DEVICE_ATTRIBUTE_DEFERRED_MAPPING_CUDA_ARRAY_SUPPORTED = 121,
  CU_DEVICE_ATTRIBUTE_CAN_USE_64_BIT_STREAM_MEM_OPS = 122,
  CU_DEVICE_ATTRIBUTE_CAN_USE_STREAM_WAIT_VALUE_NOR = 123,
  CU_DEVICE_ATTRIBUTE_DMA_BUF_SUPPORTED = 124,
  CU_DEVICE_ATTRIBUTE_IPC_EVENT_SUPPORTED = 125,
  CU_DEVICE_ATTRIBUTE_MEM_SYNC_DOMAIN_COUNT = 126,
  CU_DEVICE_ATTRIBUTE_TENSOR_MAP_ACCESS_SUPPORTED = 127,
  CU_DEVICE_ATTRIBUTE_UNIFIED_FUNCTION_POINTERS = 129,
  CU_DEVICE_ATTRIBUTE_MULTICAST_SUPPORTED = 132,
  CU_DEVICE_ATTRIBUTE_MAX,
} CUdevice_attribute;

typedef struct CUdevprop_st {
  int maxThreadsPerBlock;
  int maxThreadsDim[3];
  int maxGridSize[3];
  int sharedMemPerBlock;
  int totalConstantMemory;
  int SIMDWidth;
  int memPitch;
  int regsPerBlock;
  int clockRate;
  int textureAlign;
} CUdevprop_v1;

typedef CUdevprop_v1 CUdevprop;

typedef enum CUpointer_attribute_enum {
  CU_POINTER_ATTRIBUTE_CONTEXT = 1,
  CU_POINTER_ATTRIBUTE_MEMORY_TYPE = 2,
  CU_POINTER_ATTRIBUTE_DEVICE_POINTER = 3,
  CU_POINTER_ATTRIBUTE_HOST_POINTER = 4,
  CU_POINTER_ATTRIBUTE_P2P_TOKENS = 5,
  CU_POINTER_ATTRIBUTE_SYNC_MEMOPS = 6,
  CU_POINTER_ATTRIBUTE_BUFFER_ID = 7,
  CU_POINTER_ATTRIBUTE_IS_MANAGED = 8,
  CU_POINTER_ATTRIBUTE_DEVICE_ORDINAL = 9,
  CU_POINTER_ATTRIBUTE_IS_LEGACY_CUDA_IPC_CAPABLE = 10,
  CU_POINTER_ATTRIBUTE_RANGE_START_ADDR = 11,
  CU_POINTER_ATTRIBUTE_RANGE_SIZE = 12,
  CU_POINTER_ATTRIBUTE_MAPPED = 13,
  CU_POINTER_ATTRIBUTE_ALLOWED_HANDLE_TYPES = 14,
  CU_POINTER_ATTRIBUTE_IS_GPU_DIRECT_RDMA_CAPABLE = 15,
  CU_POINTER_ATTRIBUTE_ACCESS_FLAGS = 16,
  CU_POINTER_ATTRIBUTE_MEMPOOL_HANDLE = 17,
  CU_POINTER_ATTRIBUTE_MAPPING_SIZE = 18,
  CU_POINTER_ATTRIBUTE_MAPPING_BASE_ADDR = 19,
  CU_POINTER_ATTRIBUTE_MEMORY_BLOCK_ID = 20,
} CUpointer_attribute;

typedef enum CUfunction_attribute_enum {
  CU_FUNC_ATTRIBUTE_MAX_THREADS_PER_BLOCK = 0,
  CU_FUNC_ATTRIBUTE_SHARED_SIZE_BYTES = 1,
  CU_FUNC_ATTRIBUTE_CONST_SIZE_BYTES = 2,
  CU_FUNC_ATTRIBUTE_LOCAL_SIZE_BYTES = 3,
  CU_FUNC_ATTRIBUTE_NUM_REGS = 4,
  CU_FUNC_ATTRIBUTE_PTX_VERSION = 5,
  CU_FUNC_ATTRIBUTE_BINARY_VERSION = 6,
  CU_FUNC_ATTRIBUTE_CACHE_MODE_CA = 7,
  CU_FUNC_ATTRIBUTE_MAX_DYNAMIC_SHARED_SIZE_BYTES = 8,
  CU_FUNC_ATTRIBUTE_PREFERRED_SHARED_MEMORY_CARVEOUT = 9,
  CU_FUNC_ATTRIBUTE_CLUSTER_SIZE_MUST_BE_SET = 10,
  CU_FUNC_ATTRIBUTE_REQUIRED_CLUSTER_WIDTH = 11,
  CU_FUNC_ATTRIBUTE_REQUIRED_CLUSTER_HEIGHT = 12,
  CU_FUNC_ATTRIBUTE_REQUIRED_CLUSTER_DEPTH = 13,
  CU_FUNC_ATTRIBUTE_NON_PORTABLE_CLUSTER_SIZE_ALLOWED = 14,
  CU_FUNC_ATTRIBUTE_CLUSTER_SCHEDULING_POLICY_PREFERENCE = 15,
  CU_FUNC_ATTRIBUTE_MAX,
} CUfunction_attribute;

typedef enum CUfunc_cache_enum {
  CU_FUNC_CACHE_PREFER_NONE = 0x00,
  CU_FUNC_CACHE_PREFER_SHARED = 0x01,
  CU_FUNC_CACHE_PREFER_L1 = 0x02,
  CU_FUNC_CACHE_PREFER_EQUAL = 0x03,
} CUfunc_cache;

typedef enum CUsharedconfig_enum {
  CU_SHARED_MEM_CONFIG_DEFAULT_BANK_SIZE = 0x00,
  CU_SHARED_MEM_CONFIG_FOUR_BYTE_BANK_SIZE = 0x01,
  CU_SHARED_MEM_CONFIG_EIGHT_BYTE_BANK_SIZE = 0x02,
} CUsharedconfig;

typedef enum CUshared_carveout_enum {
  CU_SHAREDMEM_CARVEOUT_DEFAULT = -1,
  CU_SHAREDMEM_CARVEOUT_MAX_SHARED = 100,
  CU_SHAREDMEM_CARVEOUT_MAX_L1 = 0,
} CUshared_carveout;

typedef enum CUmemorytype_enum {
  CU_MEMORYTYPE_HOST = 0x01,
  CU_MEMORYTYPE_DEVICE = 0x02,
  CU_MEMORYTYPE_ARRAY = 0x03,
  CU_MEMORYTYPE_UNIFIED = 0x04,
} CUmemorytype;

typedef enum CUcomputemode_enum {
  CU_COMPUTEMODE_DEFAULT = 0,
  CU_COMPUTEMODE_PROHIBITED = 2,
  CU_COMPUTEMODE_EXCLUSIVE_PROCESS = 3,
} CUcomputemode;

typedef enum CUmem_advise_enum {
  CU_MEM_ADVISE_SET_READ_MOSTLY = 1,
  CU_MEM_ADVISE_UNSET_READ_MOSTLY = 2,
  CU_MEM_ADVISE_SET_PREFERRED_LOCATION = 3,
  CU_MEM_ADVISE_UNSET_PREFERRED_LOCATION = 4,
  CU_MEM_ADVISE_SET_ACCESSED_BY = 5,
  CU_MEM_ADVISE_UNSET_ACCESSED_BY = 6,
} CUmem_advise;

typedef enum CUmem_range_attribute_enum {
  CU_MEM_RANGE_ATTRIBUTE_READ_MOSTLY = 1,
  CU_MEM_RANGE_ATTRIBUTE_PREFERRED_LOCATION = 2,
  CU_MEM_RANGE_ATTRIBUTE_ACCESSED_BY = 3,
  CU_MEM_RANGE_ATTRIBUTE_LAST_PREFETCH_LOCATION = 4,
} CUmem_range_attribute;

typedef enum CUjit_option_enum {
  CU_JIT_MAX_REGISTERS = 0,
  CU_JIT_THREADS_PER_BLOCK = 1,
  CU_JIT_WALL_TIME = 2,
  CU_JIT_INFO_LOG_BUFFER = 3,
  CU_JIT_INFO_LOG_BUFFER_SIZE_BYTES = 4,
  CU_JIT_ERROR_LOG_BUFFER = 5,
  CU_JIT_ERROR_LOG_BUFFER_SIZE_BYTES = 6,
  CU_JIT_OPTIMIZATION_LEVEL = 7,
  CU_JIT_TARGET_FROM_CUCONTEXT = 8,
  CU_JIT_TARGET = 9,
  CU_JIT_FALLBACK_STRATEGY = 10,
  CU_JIT_GENERATE_DEBUG_INFO = 11,
  CU_JIT_LOG_VERBOSE = 12,
  CU_JIT_GENERATE_LINE_INFO = 13,
  CU_JIT_CACHE_MODE = 14,
  CU_JIT_NEW_SM3X_OPT = 15,
  CU_JIT_FAST_COMPILE = 16,
  CU_JIT_GLOBAL_SYMBOL_NAMES = 17,
  CU_JIT_GLOBAL_SYMBOL_ADDRESSES = 18,
  CU_JIT_GLOBAL_SYMBOL_COUNT = 19,
  CU_JIT_LTO = 20,
  CU_JIT_FTZ = 21,
  CU_JIT_PREC_DIV = 22,
  CU_JIT_PREC_SQRT = 23,
  CU_JIT_FMA = 24,
  CU_JIT_REFERENCED_KERNEL_NAMES = 25,
  CU_JIT_REFERENCED_KERNEL_COUNT = 26,
  CU_JIT_REFERENCED_VARIABLE_NAMES = 27,
  CU_JIT_REFERENCED_VARIABLE_COUNT = 28,
  CU_JIT_OPTIMIZE_UNUSED_DEVICE_VARIABLES = 29,
  CU_JIT_POSITION_INDEPENDENT_CODE = 30,
  CU_JIT_NUM_OPTIONS,
} CUjit_option;

typedef enum CUjit_target_enum {
  CU_TARGET_COMPUTE_30 = 30,
  CU_TARGET_COMPUTE_32 = 32,
  CU_TARGET_COMPUTE_35 = 35,
  CU_TARGET_COMPUTE_37 = 37,
  CU_TARGET_COMPUTE_50 = 50,
  CU_TARGET_COMPUTE_52 = 52,
  CU_TARGET_COMPUTE_53 = 53,
  CU_TARGET_COMPUTE_60 = 60,
  CU_TARGET_COMPUTE_61 = 61,
  CU_TARGET_COMPUTE_62 = 62,
  CU_TARGET_COMPUTE_70 = 70,
  CU_TARGET_COMPUTE_72 = 72,
  CU_TARGET_COMPUTE_75 = 75,
  CU_TARGET_COMPUTE_80 = 80,
  CU_TARGET_COMPUTE_86 = 86,
  CU_TARGET_COMPUTE_87 = 87,
  CU_TARGET_COMPUTE_89 = 89,
  CU_TARGET_COMPUTE_90 = 90,
  CU_TARGET_COMPUTE_90A = 65626,
} CUjit_target;

typedef enum CUjit_fallback_enum {
  CU_PREFER_PTX = 0,
  CU_PREFER_BINARY,
} CUjit_fallback;

typedef enum CUjit_cacheMode_enum {
  CU_JIT_CACHE_OPTION_NONE = 0,
  CU_JIT_CACHE_OPTION_CG,
  CU_JIT_CACHE_OPTION_CA,
} CUjit_cacheMode;

typedef enum CUjitInputType_enum {
  CU_JIT_INPUT_CUBIN = 0,
  CU_JIT_INPUT_PTX = 1,
  CU_JIT_INPUT_FATBINARY = 2,
  CU_JIT_INPUT_OBJECT = 3,
  CU_JIT_INPUT_LIBRARY = 4,
  CU_JIT_INPUT_NVVM = 5,
  CU_JIT_NUM_INPUT_TYPES = 6,
} CUjitInputType;

typedef struct CUlinkState_st* CUlinkState;

typedef enum CUgraphicsRegisterFlags_enum {
  CU_GRAPHICS_REGISTER_FLAGS_NONE = 0x00,
  CU_GRAPHICS_REGISTER_FLAGS_READ_ONLY = 0x01,
  CU_GRAPHICS_REGISTER_FLAGS_WRITE_DISCARD = 0x02,
  CU_GRAPHICS_REGISTER_FLAGS_SURFACE_LDST = 0x04,
  CU_GRAPHICS_REGISTER_FLAGS_TEXTURE_GATHER = 0x08,
} CUgraphicsRegisterFlags;

typedef enum CUgraphicsMapResourceFlags_enum {
  CU_GRAPHICS_MAP_RESOURCE_FLAGS_NONE = 0x00,
  CU_GRAPHICS_MAP_RESOURCE_FLAGS_READ_ONLY = 0x01,
  CU_GRAPHICS_MAP_RESOURCE_FLAGS_WRITE_DISCARD = 0x02,
} CUgraphicsMapResourceFlags;

typedef enum CUarray_cubemap_face_enum {
  CU_CUBEMAP_FACE_POSITIVE_X = 0x00,
  CU_CUBEMAP_FACE_NEGATIVE_X = 0x01,
  CU_CUBEMAP_FACE_POSITIVE_Y = 0x02,
  CU_CUBEMAP_FACE_NEGATIVE_Y = 0x03,
  CU_CUBEMAP_FACE_POSITIVE_Z = 0x04,
  CU_CUBEMAP_FACE_NEGATIVE_Z = 0x05,
} CUarray_cubemap_face;

typedef enum CUlimit_enum {
  CU_LIMIT_STACK_SIZE = 0x00,
  CU_LIMIT_PRINTF_FIFO_SIZE = 0x01,
  CU_LIMIT_MALLOC_HEAP_SIZE = 0x02,
  CU_LIMIT_DEV_RUNTIME_SYNC_DEPTH = 0x03,
  CU_LIMIT_DEV_RUNTIME_PENDING_LAUNCH_COUNT = 0x04,
  CU_LIMIT_MAX_L2_FETCH_GRANULARITY = 0x05,
  CU_LIMIT_PERSISTING_L2_CACHE_SIZE = 0x06,
  CU_LIMIT_MAX,
} CUlimit;

typedef enum CUresourcetype_enum {
  CU_RESOURCE_TYPE_ARRAY = 0x00,
  CU_RESOURCE_TYPE_MIPMAPPED_ARRAY = 0x01,
  CU_RESOURCE_TYPE_LINEAR = 0x02,
  CU_RESOURCE_TYPE_PITCH2D = 0x03,
} CUresourcetype;

typedef void (CUDA_CB *CUhostFn)(void* userData);

typedef enum CUaccessProperty_enum {
  CU_ACCESS_PROPERTY_NORMAL = 0,
  CU_ACCESS_PROPERTY_STREAMING = 1,
  CU_ACCESS_PROPERTY_PERSISTING = 2,
} CUaccessProperty;

typedef struct CUaccessPolicyWindow_st {
  void* base_ptr;
  size_t num_bytes;
  float hitRatio;
  CUaccessProperty hitProp;
  CUaccessProperty missProp;
} CUaccessPolicyWindow_v1;

typedef CUaccessPolicyWindow_v1 CUaccessPolicyWindow;

typedef struct CUDA_KERNEL_NODE_PARAMS_st {
  CUfunction func;
  unsigned int gridDimX;
  unsigned int gridDimY;
  unsigned int gridDimZ;
  unsigned int blockDimX;
  unsigned int blockDimY;
  unsigned int blockDimZ;
  unsigned int sharedMemBytes;
  void** kernelParams;
  void** extra;
} CUDA_KERNEL_NODE_PARAMS_v1;

typedef struct CUDA_KERNEL_NODE_PARAMS_v2_st {
  CUfunction func;
  unsigned int gridDimX;
  unsigned int gridDimY;
  unsigned int gridDimZ;
  unsigned int blockDimX;
  unsigned int blockDimY;
  unsigned int blockDimZ;
  unsigned int sharedMemBytes;
  void** kernelParams;
  void** extra;
  CUkernel kern;
  CUcontext ctx;
} CUDA_KERNEL_NODE_PARAMS_v2;

typedef CUDA_KERNEL_NODE_PARAMS_v2 CUDA_KERNEL_NODE_PARAMS;

typedef struct CUDA_MEMSET_NODE_PARAMS_st {
  CUdeviceptr dst;
  size_t pitch;
  unsigned int value;
  unsigned int elementSize;
  size_t width;
  size_t height;
} CUDA_MEMSET_NODE_PARAMS_v1;

typedef CUDA_MEMSET_NODE_PARAMS_v1 CUDA_MEMSET_NODE_PARAMS;

typedef struct CUDA_HOST_NODE_PARAMS_st {
  CUhostFn fn;
  void* userData;
} CUDA_HOST_NODE_PARAMS_v1;

typedef CUDA_HOST_NODE_PARAMS_v1 CUDA_HOST_NODE_PARAMS;

typedef enum CUgraphNodeType_enum {
  CU_GRAPH_NODE_TYPE_KERNEL = 0,
  CU_GRAPH_NODE_TYPE_MEMCPY = 1,
  CU_GRAPH_NODE_TYPE_MEMSET = 2,
  CU_GRAPH_NODE_TYPE_HOST = 3,
  CU_GRAPH_NODE_TYPE_GRAPH = 4,
  CU_GRAPH_NODE_TYPE_EMPTY = 5,
  CU_GRAPH_NODE_TYPE_WAIT_EVENT = 6,
  CU_GRAPH_NODE_TYPE_EVENT_RECORD = 7,
  CU_GRAPH_NODE_TYPE_EXT_SEMAS_SIGNAL = 8,
  CU_GRAPH_NODE_TYPE_EXT_SEMAS_WAIT = 9,
  CU_GRAPH_NODE_TYPE_MEM_ALLOC = 10,
  CU_GRAPH_NODE_TYPE_MEM_FREE = 11,
  CU_GRAPH_NODE_TYPE_BATCH_MEM_OP = 12,
} CUgraphNodeType;

typedef enum CUgraphInstantiateResult_enum {
  CUDA_GRAPH_INSTANTIATE_SUCCESS = 0,
  CUDA_GRAPH_INSTANTIATE_ERROR = 1,
  CUDA_GRAPH_INSTANTIATE_INVALID_STRUCTURE = 2,
  CUDA_GRAPH_INSTANTIATE_NODE_OPERATION_NOT_SUPPORTED = 3,
  CUDA_GRAPH_INSTANTIATE_MULTIPLE_CTXS_NOT_SUPPORTED = 4,
} CUgraphInstantiateResult;

typedef struct CUDA_GRAPH_INSTANTIATE_PARAMS_st {
  cuuint64_t flags;
  CUstream hUploadStream;
  CUgraphNode hErrNode_out;
  CUgraphInstantiateResult result_out;
} CUDA_GRAPH_INSTANTIATE_PARAMS;

typedef enum CUsynchronizationPolicy_enum {
  CU_SYNC_POLICY_AUTO = 1,
  CU_SYNC_POLICY_SPIN = 2,
  CU_SYNC_POLICY_YIELD = 3,
  CU_SYNC_POLICY_BLOCKING_SYNC = 4,
} CUsynchronizationPolicy;

typedef enum CUclusterSchedulingPolicy_enum {
  CU_CLUSTER_SCHEDULING_POLICY_DEFAULT = 0,
  CU_CLUSTER_SCHEDULING_POLICY_SPREAD = 1,
  CU_CLUSTER_SCHEDULING_POLICY_LOAD_BALANCING = 2,
} CUclusterSchedulingPolicy;

typedef enum CUlaunchMemSyncDomain_enum {
  CU_LAUNCH_MEM_SYNC_DOMAIN_DEFAULT = 0,
  CU_LAUNCH_MEM_SYNC_DOMAIN_REMOTE = 1,
} CUlaunchMemSyncDomain;

typedef struct CUlaunchMemSyncDomainMap_st {
  unsigned char default_;
  unsigned char remote;
} CUlaunchMemSyncDomainMap;

typedef enum CUlaunchAttributeID_enum {
  CU_LAUNCH_ATTRIBUTE_IGNORE = 0,
  CU_LAUNCH_ATTRIBUTE_ACCESS_POLICY_WINDOW = 1,
  CU_LAUNCH_ATTRIBUTE_COOPERATIVE = 2,
  CU_LAUNCH_ATTRIBUTE_SYNCHRONIZATION_POLICY = 3,
  CU_LAUNCH_ATTRIBUTE_CLUSTER_DIMENSION = 4,
  CU_LAUNCH_ATTRIBUTE_CLUSTER_SCHEDULING_POLICY_PREFERENCE = 5,
  CU_LAUNCH_ATTRIBUTE_PROGRAMMATIC_STREAM_SERIALIZATION = 6,
  CU_LAUNCH_ATTRIBUTE_PROGRAMMATIC_EVENT = 7,
  CU_LAUNCH_ATTRIBUTE_PRIORITY = 8,
  CU_LAUNCH_ATTRIBUTE_MEM_SYNC_DOMAIN_MAP = 9,
  CU_LAUNCH_ATTRIBUTE_MEM_SYNC_DOMAIN = 10,
} CUlaunchAttributeID;

typedef union CUlaunchAttributeValue_union {
  char pad[64];
  CUaccessPolicyWindow accessPolicyWindow;
  int cooperative;
  CUsynchronizationPolicy syncPolicy;
  struct {
    unsigned int x;
    unsigned int y;
    unsigned int z;
  } clusterDim;
  CUclusterSchedulingPolicy clusterSchedulingPolicyPreference;
  int programmaticStreamSerializationAllowed;
  struct {
    CUevent event;
    int flags;
    int triggerAtBlockStart;
  } programmaticEvent;
  int priority;
  CUlaunchMemSyncDomainMap memSyncDomainMap;
  CUlaunchMemSyncDomain memSyncDomain;
} CUlaunchAttributeValue;

typedef struct CUlaunchAttribute_st {
  CUlaunchAttributeID id;
  char pad[8 - sizeof(CUlaunchAttributeID)];
  CUlaunchAttributeValue value;
} CUlaunchAttribute;

typedef struct CUlaunchConfig_st {
  unsigned int gridDimX;
  unsigned int gridDimY;
  unsigned int gridDimZ;
  unsigned int blockDimX;
  unsigned int blockDimY;
  unsigned int blockDimZ;
  unsigned int sharedMemBytes;
  CUstream hStream;
  CUlaunchAttribute* attrs;
  unsigned int numAttrs;
} CUlaunchConfig;

typedef CUlaunchAttributeID CUkernelNodeAttrID;
typedef CUlaunchAttributeValue CUkernelNodeAttrValue_v1;
typedef CUkernelNodeAttrValue_v1 CUkernelNodeAttrValue;

typedef enum CUstreamCaptureStatus_enum {
  CU_STREAM_CAPTURE_STATUS_NONE = 0,
  CU_STREAM_CAPTURE_STATUS_ACTIVE = 1,
  CU_STREAM_CAPTURE_STATUS_INVALIDATED = 2,
} CUstreamCaptureStatus;

typedef enum CUstreamCaptureMode_enum {
  CU_STREAM_CAPTURE_MODE_GLOBAL = 0,
  CU_STREAM_CAPTURE_MODE_THREAD_LOCAL = 1,
  CU_STREAM_CAPTURE_MODE_RELAXED = 2,
} CUstreamCaptureMode;

typedef CUlaunchAttributeID CUstreamAttrID;
typedef CUlaunchAttributeValue CUstreamAttrValue_v1;
typedef CUstreamAttrValue_v1 CUstreamAttrValue;

typedef enum CUdriverProcAddress_flags_enum {
  CU_GET_PROC_ADDRESS_DEFAULT = 0,
  CU_GET_PROC_ADDRESS_LEGACY_STREAM = (1 << 0),
  CU_GET_PROC_ADDRESS_PER_THREAD_DEFAULT_STREAM = (1 << 1),
} CUdriverProcAddress_flags;

typedef enum CUdriverProcAddressQueryResult_enum {
  CU_GET_PROC_ADDRESS_SUCCESS = 0,
  CU_GET_PROC_ADDRESS_SYMBOL_NOT_FOUND = 1,
  CU_GET_PROC_ADDRESS_VERSION_NOT_SUFFICIENT = 2,
} CUdriverProcAddressQueryResult;

typedef enum CUexecAffinityType_enum {
  CU_EXEC_AFFINITY_TYPE_SM_COUNT = 0,
  CU_EXEC_AFFINITY_TYPE_MAX,
} CUexecAffinityType;

typedef struct CUexecAffinitySmCount_st {
  unsigned int val;
} CUexecAffinitySmCount_v1;

typedef CUexecAffinitySmCount_v1 CUexecAffinitySmCount;

typedef struct CUexecAffinityParam_st {
  CUexecAffinityType type;
  union {
    CUexecAffinitySmCount smCount;
  } param;
} CUexecAffinityParam_v1;

typedef CUexecAffinityParam_v1 CUexecAffinityParam;

typedef enum CUlibraryOption_enum {
  CU_LIBRARY_HOST_UNIVERSAL_FUNCTION_AND_DATA_TABLE = 0,
  CU_LIBRARY_BINARY_IS_PRESERVED = 1,
  CU_LIBRARY_NUM_OPTIONS,
} CUlibraryOption;

typedef struct CUlibraryHostUniversalFunctionAndDataTable_st {
  void* functionTable;
  size_t functionWindowSize;
  void* dataTable;
  size_t dataWindowSize;
} CUlibraryHostUniversalFunctionAndDataTable;

typedef enum cudaError_enum {
  CUDA_SUCCESS = 0,
  CUDA_ERROR_INVALID_VALUE = 1,
  CUDA_ERROR_OUT_OF_MEMORY = 2,
  CUDA_ERROR_NOT_INITIALIZED = 3,
  CUDA_ERROR_DEINITIALIZED = 4,
  CUDA_ERROR_PROFILER_DISABLED = 5,
  CUDA_ERROR_PROFILER_NOT_INITIALIZED = 6,
  CUDA_ERROR_PROFILER_ALREADY_STARTED = 7,
  CUDA_ERROR_PROFILER_ALREADY_STOPPED = 8,
  CUDA_ERROR_STUB_LIBRARY = 34,
  CUDA_ERROR_DEVICE_UNAVAILABLE = 46,
  CUDA_ERROR_NO_DEVICE = 100,
  CUDA_ERROR_INVALID_DEVICE = 101,
  CUDA_ERROR_DEVICE_NOT_LICENSED = 102,
  CUDA_ERROR_INVALID_IMAGE = 200,
  CUDA_ERROR_INVALID_CONTEXT = 201,
  CUDA_ERROR_CONTEXT_ALREADY_CURRENT = 202,
  CUDA_ERROR_MAP_FAILED = 205,
  CUDA_ERROR_UNMAP_FAILED = 206,
  CUDA_ERROR_ARRAY_IS_MAPPED = 207,
  CUDA_ERROR_ALREADY_MAPPED = 208,
  CUDA_ERROR_NO_BINARY_FOR_GPU = 209,
  CUDA_ERROR_ALREADY_ACQUIRED = 210,
  CUDA_ERROR_NOT_MAPPED = 211,
  CUDA_ERROR_NOT_MAPPED_AS_ARRAY = 212,
  CUDA_ERROR_NOT_MAPPED_AS_POINTER = 213,
  CUDA_ERROR_ECC_UNCORRECTABLE = 214,
  CUDA_ERROR_UNSUPPORTED_LIMIT = 215,
  CUDA_ERROR_CONTEXT_ALREADY_IN_USE = 216,
  CUDA_ERROR_PEER_ACCESS_UNSUPPORTED = 217,
  CUDA_ERROR_INVALID_PTX = 218,
  CUDA_ERROR_INVALID_GRAPHICS_CONTEXT = 219,
  CUDA_ERROR_NVLINK_UNCORRECTABLE = 220,
  CUDA_ERROR_JIT_COMPILER_NOT_FOUND = 221,
  CUDA_ERROR_UNSUPPORTED_PTX_VERSION = 222,
  CUDA_ERROR_JIT_COMPILATION_DISABLED = 223,
  CUDA_ERROR_UNSUPPORTED_EXEC_AFFINITY = 224,
  CUDA_ERROR_UNSUPPORTED_DEVSIDE_SYNC = 225,
  CUDA_ERROR_INVALID_SOURCE = 300,
  CUDA_ERROR_FILE_NOT_FOUND = 301,
  CUDA_ERROR_SHARED_OBJECT_SYMBOL_NOT_FOUND = 302,
  CUDA_ERROR_SHARED_OBJECT_INIT_FAILED = 303,
  CUDA_ERROR_OPERATING_SYSTEM = 304,
  CUDA_ERROR_INVALID_HANDLE = 400,
  CUDA_ERROR_ILLEGAL_STATE = 401,
  CUDA_ERROR_NOT_FOUND = 500,
  CUDA_ERROR_NOT_READY = 600,
  CUDA_ERROR_ILLEGAL_ADDRESS = 700,
  CUDA_ERROR_LAUNCH_OUT_OF_RESOURCES = 701,
  CUDA_ERROR_LAUNCH_TIMEOUT = 702,
  CUDA_ERROR_LAUNCH_INCOMPATIBLE_TEXTURING = 703,
  CUDA_ERROR_PEER_ACCESS_ALREADY_ENABLED = 704,
  CUDA_ERROR_PEER_ACCESS_NOT_ENABLED = 705,
  CUDA_ERROR_PRIMARY_CONTEXT_ACTIVE = 708,
  CUDA_ERROR_CONTEXT_IS_DESTROYED = 709,
  CUDA_ERROR_ASSERT = 710,
  CUDA_ERROR_TOO_MANY_PEERS = 711,
  CUDA_ERROR_HOST_MEMORY_ALREADY_REGISTERED = 712,
  CUDA_ERROR_HOST_MEMORY_NOT_REGISTERED = 713,
  CUDA_ERROR_HARDWARE_STACK_ERROR = 714,
  CUDA_ERROR_ILLEGAL_INSTRUCTION = 715,
  CUDA_ERROR_MISALIGNED_ADDRESS = 716,
  CUDA_ERROR_INVALID_ADDRESS_SPACE = 717,
  CUDA_ERROR_INVALID_PC = 718,
  CUDA_ERROR_LAUNCH_FAILED = 719,
  CUDA_ERROR_COOPERATIVE_LAUNCH_TOO_LARGE = 720,
  CUDA_ERROR_NOT_PERMITTED = 800,
  CUDA_ERROR_NOT_SUPPORTED = 801,
  CUDA_ERROR_SYSTEM_NOT_READY = 802,
  CUDA_ERROR_SYSTEM_DRIVER_MISMATCH = 803,
  CUDA_ERROR_COMPAT_NOT_SUPPORTED_ON_DEVICE = 804,
  CUDA_ERROR_MPS_CONNECTION_FAILED = 805,
  CUDA_ERROR_MPS_RPC_FAILURE = 806,
  CUDA_ERROR_MPS_SERVER_NOT_READY = 807,
  CUDA_ERROR_MPS_MAX_CLIENTS_REACHED = 808,
  CUDA_ERROR_MPS_MAX_CONNECTIONS_REACHED = 809,
  CUDA_ERROR_MPS_CLIENT_TERMINATED = 810,
  CUDA_ERROR_CDP_NOT_SUPPORTED = 811,
  CUDA_ERROR_CDP_VERSION_MISMATCH = 812,
  CUDA_ERROR_STREAM_CAPTURE_UNSUPPORTED = 900,
  CUDA_ERROR_STREAM_CAPTURE_INVALIDATED = 901,
  CUDA_ERROR_STREAM_CAPTURE_MERGE = 902,
  CUDA_ERROR_STREAM_CAPTURE_UNMATCHED = 903,
  CUDA_ERROR_STREAM_CAPTURE_UNJOINED = 904,
  CUDA_ERROR_STREAM_CAPTURE_ISOLATION = 905,
  CUDA_ERROR_STREAM_CAPTURE_IMPLICIT = 906,
  CUDA_ERROR_CAPTURED_EVENT = 907,
  CUDA_ERROR_STREAM_CAPTURE_WRONG_THREAD = 908,
  CUDA_ERROR_TIMEOUT = 909,
  CUDA_ERROR_GRAPH_EXEC_UPDATE_FAILURE = 910,
  CUDA_ERROR_EXTERNAL_DEVICE = 911,
  CUDA_ERROR_INVALID_CLUSTER_SIZE = 912,
  CUDA_ERROR_UNKNOWN = 999,
} CUresult;

typedef enum CUdevice_P2PAttribute_enum {
  CU_DEVICE_P2P_ATTRIBUTE_PERFORMANCE_RANK = 0x01,
  CU_DEVICE_P2P_ATTRIBUTE_ACCESS_SUPPORTED = 0x02,
  CU_DEVICE_P2P_ATTRIBUTE_NATIVE_ATOMIC_SUPPORTED = 0x03,
  CU_DEVICE_P2P_ATTRIBUTE_ACCESS_ACCESS_SUPPORTED = 0x04,
  CU_DEVICE_P2P_ATTRIBUTE_CUDA_ARRAY_ACCESS_SUPPORTED = 0x04,
} CUdevice_P2PAttribute;

typedef void (CUDA_CB *CUstreamCallback)(CUstream hStream, CUresult status, void* userData);
typedef size_t (CUDA_CB *CUoccupancyB2DSize)(int blockSize);

typedef struct CUDA_MEMCPY2D_st {
  size_t srcXInBytes;
  size_t srcY;
  CUmemorytype srcMemoryType;
  const void* srcHost;
  CUdeviceptr srcDevice;
  CUarray srcArray;
  size_t srcPitch;
  size_t dstXInBytes;
  size_t dstY;
  CUmemorytype dstMemoryType;
  void* dstHost;
  CUdeviceptr dstDevice;
  CUarray dstArray;
  size_t dstPitch;
  size_t WidthInBytes;
  size_t Height;
} CUDA_MEMCPY2D_v2;

typedef CUDA_MEMCPY2D_v2 CUDA_MEMCPY2D;

typedef struct CUDA_MEMCPY3D_st {
  size_t srcXInBytes;
  size_t srcY;
  size_t srcZ;
  size_t srcLOD;
  CUmemorytype srcMemoryType;
  const void* srcHost;
  CUdeviceptr srcDevice;
  CUarray srcArray;
  void* reserved0;
  size_t srcPitch;
  size_t srcHeight;
  size_t dstXInBytes;
  size_t dstY;
  size_t dstZ;
  size_t dstLOD;
  CUmemorytype dstMemoryType;
  void* dstHost;
  CUdeviceptr dstDevice;
  CUarray dstArray;
  void* reserved1;
  size_t dstPitch;
  size_t dstHeight;
  size_t WidthInBytes;
  size_t Height;
  size_t Depth;
} CUDA_MEMCPY3D_v2;

typedef CUDA_MEMCPY3D_v2 CUDA_MEMCPY3D;

typedef struct CUDA_MEMCPY3D_PEER_st {
  size_t srcXInBytes;
  size_t srcY;
  size_t srcZ;
  size_t srcLOD;
  CUmemorytype srcMemoryType;
  const void* srcHost;
  CUdeviceptr srcDevice;
  CUarray srcArray;
  CUcontext srcContext;
  size_t srcPitch;
  size_t srcHeight;
  size_t dstXInBytes;
  size_t dstY;
  size_t dstZ;
  size_t dstLOD;
  CUmemorytype dstMemoryType;
  void* dstHost;
  CUdeviceptr dstDevice;
  CUarray dstArray;
  CUcontext dstContext;
  size_t dstPitch;
  size_t dstHeight;
  size_t WidthInBytes;
  size_t Height;
  size_t Depth;
} CUDA_MEMCPY3D_PEER_v1;

typedef CUDA_MEMCPY3D_PEER_v1 CUDA_MEMCPY3D_PEER;

typedef struct CUDA_ARRAY_DESCRIPTOR_st {
  size_t Width;
  size_t Height;
  CUarray_format Format;
  unsigned int NumChannels;
} CUDA_ARRAY_DESCRIPTOR_v2;

typedef CUDA_ARRAY_DESCRIPTOR_v2 CUDA_ARRAY_DESCRIPTOR;

typedef struct CUDA_ARRAY3D_DESCRIPTOR_st {
  size_t Width;
  size_t Height;
  size_t Depth;
  CUarray_format Format;
  unsigned int NumChannels;
  unsigned int Flags;
} CUDA_ARRAY3D_DESCRIPTOR_v2;

typedef CUDA_ARRAY3D_DESCRIPTOR_v2 CUDA_ARRAY3D_DESCRIPTOR;

typedef struct CUDA_ARRAY_SPARSE_PROPERTIES_st {
  struct {
    unsigned int width;
    unsigned int height;
    unsigned int depth;
  } tileExtent;
  unsigned int miptailFirstLevel;
  unsigned long long miptailSize;
  unsigned int flags;
  unsigned int reserved[4];
} CUDA_ARRAY_SPARSE_PROPERTIES_v1;

typedef CUDA_ARRAY_SPARSE_PROPERTIES_v1 CUDA_ARRAY_SPARSE_PROPERTIES;

typedef struct CUDA_ARRAY_MEMORY_REQUIREMENTS_st {
  size_t size;
  size_t alignment;
  unsigned int reserved[4];
} CUDA_ARRAY_MEMORY_REQUIREMENTS_v1;

typedef CUDA_ARRAY_MEMORY_REQUIREMENTS_v1 CUDA_ARRAY_MEMORY_REQUIREMENTS;

typedef struct CUDA_RESOURCE_DESC_st {
  CUresourcetype resType;
  union {
    struct {
      CUarray hArray;
    } array;
    struct {
      CUmipmappedArray hMipmappedArray;
    } mipmap;
    struct {
      CUdeviceptr devPtr;
      CUarray_format format;
      unsigned int numChannels;
      size_t sizeInBytes;
    } linear;
    struct {
      CUdeviceptr devPtr;
      CUarray_format format;
      unsigned int numChannels;
      size_t width;
      size_t height;
      size_t pitchInBytes;
    } pitch2D;
    struct {
      int reserved[32];
    } reserved;
  } res;
  unsigned int flags;
} CUDA_RESOURCE_DESC_v1;

typedef CUDA_RESOURCE_DESC_v1 CUDA_RESOURCE_DESC;

typedef struct CUDA_TEXTURE_DESC_st {
  CUaddress_mode addressMode[3];
  CUfilter_mode filterMode;
  unsigned int flags;
  unsigned int maxAnisotropy;
  CUfilter_mode mipmapFilterMode;
  float mipmapLevelBias;
  float minMipmapLevelClamp;
  float maxMipmapLevelClamp;
  float borderColor[4];
  int reserved[12];
} CUDA_TEXTURE_DESC_v1;

typedef CUDA_TEXTURE_DESC_v1 CUDA_TEXTURE_DESC;

typedef enum CUresourceViewFormat_enum {
  CU_RES_VIEW_FORMAT_NONE = 0x00,
  CU_RES_VIEW_FORMAT_UINT_1X8 = 0x01,
  CU_RES_VIEW_FORMAT_UINT_2X8 = 0x02,
  CU_RES_VIEW_FORMAT_UINT_4X8 = 0x03,
  CU_RES_VIEW_FORMAT_SINT_1X8 = 0x04,
  CU_RES_VIEW_FORMAT_SINT_2X8 = 0x05,
  CU_RES_VIEW_FORMAT_SINT_4X8 = 0x06,
  CU_RES_VIEW_FORMAT_UINT_1X16 = 0x07,
  CU_RES_VIEW_FORMAT_UINT_2X16 = 0x08,
  CU_RES_VIEW_FORMAT_UINT_4X16 = 0x09,
  CU_RES_VIEW_FORMAT_SINT_1X16 = 0x0a,
  CU_RES_VIEW_FORMAT_SINT_2X16 = 0x0b,
  CU_RES_VIEW_FORMAT_SINT_4X16 = 0x0c,
  CU_RES_VIEW_FORMAT_UINT_1X32 = 0x0d,
  CU_RES_VIEW_FORMAT_UINT_2X32 = 0x0e,
  CU_RES_VIEW_FORMAT_UINT_4X32 = 0x0f,
  CU_RES_VIEW_FORMAT_SINT_1X32 = 0x10,
  CU_RES_VIEW_FORMAT_SINT_2X32 = 0x11,
  CU_RES_VIEW_FORMAT_SINT_4X32 = 0x12,
  CU_RES_VIEW_FORMAT_FLOAT_1X16 = 0x13,
  CU_RES_VIEW_FORMAT_FLOAT_2X16 = 0x14,
  CU_RES_VIEW_FORMAT_FLOAT_4X16 = 0x15,
  CU_RES_VIEW_FORMAT_FLOAT_1X32 = 0x16,
  CU_RES_VIEW_FORMAT_FLOAT_2X32 = 0x17,
  CU_RES_VIEW_FORMAT_FLOAT_4X32 = 0x18,
  CU_RES_VIEW_FORMAT_UNSIGNED_BC1 = 0x19,
  CU_RES_VIEW_FORMAT_UNSIGNED_BC2 = 0x1a,
  CU_RES_VIEW_FORMAT_UNSIGNED_BC3 = 0x1b,
  CU_RES_VIEW_FORMAT_UNSIGNED_BC4 = 0x1c,
  CU_RES_VIEW_FORMAT_SIGNED_BC4 = 0x1d,
  CU_RES_VIEW_FORMAT_UNSIGNED_BC5 = 0x1e,
  CU_RES_VIEW_FORMAT_SIGNED_BC5 = 0x1f,
  CU_RES_VIEW_FORMAT_UNSIGNED_BC6H = 0x20,
  CU_RES_VIEW_FORMAT_SIGNED_BC6H = 0x21,
  CU_RES_VIEW_FORMAT_UNSIGNED_BC7 = 0x22,
} CUresourceViewFormat;

typedef struct CUDA_RESOURCE_VIEW_DESC_st {
  CUresourceViewFormat format;
  size_t width;
  size_t height;
  size_t depth;
  unsigned int firstMipmapLevel;
  unsigned int lastMipmapLevel;
  unsigned int firstLayer;
  unsigned int lastLayer;
  unsigned int reserved[16];
} CUDA_RESOURCE_VIEW_DESC_v1;

typedef CUDA_RESOURCE_VIEW_DESC_v1 CUDA_RESOURCE_VIEW_DESC;

typedef struct CUtensorMap_st {
#if defined(__cplusplus) && ( __cplusplus >= 201103L)
    alignas(64)
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
    _Alignas(64)
#endif
  cuuint64_t opaque[16];
} CUtensorMap;

typedef enum CUtensorMapDataType_enum {
  CU_TENSOR_MAP_DATA_TYPE_UINT8 = 0,
  CU_TENSOR_MAP_DATA_TYPE_UINT16,
  CU_TENSOR_MAP_DATA_TYPE_UINT32,
  CU_TENSOR_MAP_DATA_TYPE_INT32,
  CU_TENSOR_MAP_DATA_TYPE_UINT64,
  CU_TENSOR_MAP_DATA_TYPE_INT64,
  CU_TENSOR_MAP_DATA_TYPE_FLOAT16,
  CU_TENSOR_MAP_DATA_TYPE_FLOAT32,
  CU_TENSOR_MAP_DATA_TYPE_FLOAT64,
  CU_TENSOR_MAP_DATA_TYPE_BFLOAT16,
  CU_TENSOR_MAP_DATA_TYPE_FLOAT32_FTZ,
  CU_TENSOR_MAP_DATA_TYPE_TFLOAT32,
  CU_TENSOR_MAP_DATA_TYPE_TFLOAT32_FTZ,
} CUtensorMapDataType;

typedef enum CUtensorMapInterleave_enum {
  CU_TENSOR_MAP_INTERLEAVE_NONE = 0,
  CU_TENSOR_MAP_INTERLEAVE_16B,
  CU_TENSOR_MAP_INTERLEAVE_32B,
} CUtensorMapInterleave;

typedef enum CUtensorMapSwizzle_enum {
  CU_TENSOR_MAP_SWIZZLE_NONE = 0,
  CU_TENSOR_MAP_SWIZZLE_32B,
  CU_TENSOR_MAP_SWIZZLE_64B,
  CU_TENSOR_MAP_SWIZZLE_128B,
} CUtensorMapSwizzle;

typedef enum CUtensorMapL2promotion_enum {
  CU_TENSOR_MAP_L2_PROMOTION_NONE = 0,
  CU_TENSOR_MAP_L2_PROMOTION_L2_64B,
  CU_TENSOR_MAP_L2_PROMOTION_L2_128B,
  CU_TENSOR_MAP_L2_PROMOTION_L2_256B,
} CUtensorMapL2promotion;

typedef enum CUtensorMapFloatOOBfill_enum {
  CU_TENSOR_MAP_FLOAT_OOB_FILL_NONE = 0,
  CU_TENSOR_MAP_FLOAT_OOB_FILL_NAN_REQUEST_ZERO_FMA,
} CUtensorMapFloatOOBfill;

typedef struct CUDA_POINTER_ATTRIBUTE_P2P_TOKENS_st {
  unsigned long long p2pToken;
  unsigned int vaSpaceToken;
} CUDA_POINTER_ATTRIBUTE_P2P_TOKENS_v1;

typedef CUDA_POINTER_ATTRIBUTE_P2P_TOKENS_v1 CUDA_POINTER_ATTRIBUTE_P2P_TOKENS;

typedef enum CUDA_POINTER_ATTRIBUTE_ACCESS_FLAGS_enum {
  CU_POINTER_ATTRIBUTE_ACCESS_FLAG_NONE = 0x0,
  CU_POINTER_ATTRIBUTE_ACCESS_FLAG_READ = 0x1,
  CU_POINTER_ATTRIBUTE_ACCESS_FLAG_READWRITE = 0x3,
} CUDA_POINTER_ATTRIBUTE_ACCESS_FLAGS;

typedef struct CUDA_LAUNCH_PARAMS_st {
  CUfunction function;
  unsigned int gridDimX;
  unsigned int gridDimY;
  unsigned int gridDimZ;
  unsigned int blockDimX;
  unsigned int blockDimY;
  unsigned int blockDimZ;
  unsigned int sharedMemBytes;
  CUstream hStream;
  void** kernelParams;
} CUDA_LAUNCH_PARAMS_v1;

typedef CUDA_LAUNCH_PARAMS_v1 CUDA_LAUNCH_PARAMS;

typedef enum CUexternalMemoryHandleType_enum {
  CU_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD = 1,
  CU_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32 = 2,
  CU_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_KMT = 3,
  CU_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_HEAP = 4,
  CU_EXTERNAL_MEMORY_HANDLE_TYPE_D3D12_RESOURCE = 5,
  CU_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_RESOURCE = 6,
  CU_EXTERNAL_MEMORY_HANDLE_TYPE_D3D11_RESOURCE_KMT = 7,
  CU_EXTERNAL_MEMORY_HANDLE_TYPE_NVSCIBUF = 8,
} CUexternalMemoryHandleType;

typedef struct CUDA_EXTERNAL_MEMORY_HANDLE_DESC_st {
  CUexternalMemoryHandleType type;
  union {
    int fd;
    struct {
      void* handle;
      const void* name;
    } win32;
    const void* nvSciBufObject;
  } handle;
  unsigned long long size;
  unsigned int flags;
  unsigned int reserved[16];
} CUDA_EXTERNAL_MEMORY_HANDLE_DESC_v1;

typedef CUDA_EXTERNAL_MEMORY_HANDLE_DESC_v1 CUDA_EXTERNAL_MEMORY_HANDLE_DESC;

typedef struct CUDA_EXTERNAL_MEMORY_BUFFER_DESC_st {
  unsigned long long offset;
  unsigned long long size;
  unsigned int flags;
  unsigned int reserved[16];
} CUDA_EXTERNAL_MEMORY_BUFFER_DESC_v1;

typedef CUDA_EXTERNAL_MEMORY_BUFFER_DESC_v1 CUDA_EXTERNAL_MEMORY_BUFFER_DESC;

typedef struct CUDA_EXTERNAL_MEMORY_MIPMAPPED_ARRAY_DESC_st {
  unsigned long long offset;
  CUDA_ARRAY3D_DESCRIPTOR arrayDesc;
  unsigned int numLevels;
  unsigned int reserved[16];
} CUDA_EXTERNAL_MEMORY_MIPMAPPED_ARRAY_DESC_v1;

typedef CUDA_EXTERNAL_MEMORY_MIPMAPPED_ARRAY_DESC_v1 CUDA_EXTERNAL_MEMORY_MIPMAPPED_ARRAY_DESC;

typedef enum CUexternalSemaphoreHandleType_enum {
  CU_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_FD = 1,
  CU_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32 = 2,
  CU_EXTERNAL_SEMAPHORE_HANDLE_TYPE_OPAQUE_WIN32_KMT = 3,
  CU_EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D12_FENCE = 4,
  CU_EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D11_FENCE = 5,
  CU_EXTERNAL_SEMAPHORE_HANDLE_TYPE_NVSCISYNC = 6,
  CU_EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D11_KEYED_MUTEX = 7,
  CU_EXTERNAL_SEMAPHORE_HANDLE_TYPE_D3D11_KEYED_MUTEX_KMT = 8,
  CU_EXTERNAL_SEMAPHORE_HANDLE_TYPE_TIMELINE_SEMAPHORE_FD = 9,
  CU_EXTERNAL_SEMAPHORE_HANDLE_TYPE_TIMELINE_SEMAPHORE_WIN32 = 10,
} CUexternalSemaphoreHandleType;

typedef struct CUDA_EXTERNAL_SEMAPHORE_HANDLE_DESC_st {
  CUexternalSemaphoreHandleType type;
  union {
    int fd;
    struct {
      void* handle;
      const void* name;
    } win32;
    const void* nvSciSyncObj;
  } handle;
  unsigned int flags;
  unsigned int reserved[16];
} CUDA_EXTERNAL_SEMAPHORE_HANDLE_DESC_v1;

typedef CUDA_EXTERNAL_SEMAPHORE_HANDLE_DESC_v1 CUDA_EXTERNAL_SEMAPHORE_HANDLE_DESC;

typedef struct CUDA_EXTERNAL_SEMAPHORE_SIGNAL_PARAMS_st {
  struct {
    struct {
      unsigned long long value;
    } fence;
    union {
      void* fence;
      unsigned long long reserved;
    } nvSciSync;
    struct {
      unsigned long long key;
    } keyedMutex;
    unsigned int reserved[12];
  } params;
  unsigned int flags;
  unsigned int reserved[16];
} CUDA_EXTERNAL_SEMAPHORE_SIGNAL_PARAMS_v1;

typedef CUDA_EXTERNAL_SEMAPHORE_SIGNAL_PARAMS_v1 CUDA_EXTERNAL_SEMAPHORE_SIGNAL_PARAMS;

typedef struct CUDA_EXTERNAL_SEMAPHORE_WAIT_PARAMS_st {
  struct {
    struct {
      unsigned long long value;
    } fence;
    union {
      void* fence;
      unsigned long long reserved;
    } nvSciSync;
    struct {
      unsigned long long key;
      unsigned int timeoutMs;
    } keyedMutex;
    unsigned int reserved[10];
  } params;
  unsigned int flags;
  unsigned int reserved[16];
} CUDA_EXTERNAL_SEMAPHORE_WAIT_PARAMS_v1;

typedef CUDA_EXTERNAL_SEMAPHORE_WAIT_PARAMS_v1 CUDA_EXTERNAL_SEMAPHORE_WAIT_PARAMS;

typedef struct CUDA_EXT_SEM_SIGNAL_NODE_PARAMS_st {
  CUexternalSemaphore* extSemArray;
  const CUDA_EXTERNAL_SEMAPHORE_SIGNAL_PARAMS* paramsArray;
  unsigned int numExtSems;
} CUDA_EXT_SEM_SIGNAL_NODE_PARAMS_v1;

typedef CUDA_EXT_SEM_SIGNAL_NODE_PARAMS_v1 CUDA_EXT_SEM_SIGNAL_NODE_PARAMS;

typedef struct CUDA_EXT_SEM_WAIT_NODE_PARAMS_st {
  CUexternalSemaphore* extSemArray;
  const CUDA_EXTERNAL_SEMAPHORE_WAIT_PARAMS* paramsArray;
  unsigned int numExtSems;
} CUDA_EXT_SEM_WAIT_NODE_PARAMS_v1;

typedef CUDA_EXT_SEM_WAIT_NODE_PARAMS_v1 CUDA_EXT_SEM_WAIT_NODE_PARAMS;
typedef unsigned long long CUmemGenericAllocationHandle_v1;
typedef CUmemGenericAllocationHandle_v1 CUmemGenericAllocationHandle;

typedef enum CUmemAllocationHandleType_enum {
  CU_MEM_HANDLE_TYPE_NONE = 0x0,
  CU_MEM_HANDLE_TYPE_POSIX_FILE_DESCRIPTOR = 0x1,
  CU_MEM_HANDLE_TYPE_WIN32 = 0x2,
  CU_MEM_HANDLE_TYPE_WIN32_KMT = 0x4,
  CU_MEM_HANDLE_TYPE_MAX = 0x7FFFFFFF,
} CUmemAllocationHandleType;

typedef enum CUmemAccess_flags_enum {
  CU_MEM_ACCESS_FLAGS_PROT_NONE = 0x0,
  CU_MEM_ACCESS_FLAGS_PROT_READ = 0x1,
  CU_MEM_ACCESS_FLAGS_PROT_READWRITE = 0x3,
  CU_MEM_ACCESS_FLAGS_PROT_MAX = 0x7FFFFFFF,
} CUmemAccess_flags;

typedef enum CUmemLocationType_enum {
  CU_MEM_LOCATION_TYPE_INVALID = 0x0,
  CU_MEM_LOCATION_TYPE_DEVICE = 0x1,
  CU_MEM_LOCATION_TYPE_MAX = 0x7FFFFFFF,
} CUmemLocationType;

typedef enum CUmemAllocationType_enum {
  CU_MEM_ALLOCATION_TYPE_INVALID = 0x0,
  CU_MEM_ALLOCATION_TYPE_PINNED = 0x1,
  CU_MEM_ALLOCATION_TYPE_MAX = 0x7FFFFFFF,
} CUmemAllocationType;

typedef enum CUmemAllocationGranularity_flags_enum {
  CU_MEM_ALLOC_GRANULARITY_MINIMUM = 0x0,
  CU_MEM_ALLOC_GRANULARITY_RECOMMENDED = 0x1,
} CUmemAllocationGranularity_flags;

typedef enum CUmemRangeHandleType_enum {
  CU_MEM_RANGE_HANDLE_TYPE_DMA_BUF_FD = 0x1,
  CU_MEM_RANGE_HANDLE_TYPE_MAX = 0x7FFFFFFF,
} CUmemRangeHandleType;

typedef enum CUarraySparseSubresourceType_enum {
  CU_ARRAY_SPARSE_SUBRESOURCE_TYPE_SPARSE_LEVEL = 0,
  CU_ARRAY_SPARSE_SUBRESOURCE_TYPE_MIPTAIL = 1,
} CUarraySparseSubresourceType;

typedef enum CUmemOperationType_enum {
  CU_MEM_OPERATION_TYPE_MAP = 1,
  CU_MEM_OPERATION_TYPE_UNMAP = 2,
} CUmemOperationType;

typedef enum CUmemHandleType_enum {
  CU_MEM_HANDLE_TYPE_GENERIC = 0,
} CUmemHandleType;

typedef struct CUarrayMapInfo_st {
  CUresourcetype resourceType;
  union {
    CUmipmappedArray mipmap;
    CUarray array;
  } resource;
  CUarraySparseSubresourceType subresourceType;
  union {
    struct {
      unsigned int level;
      unsigned int layer;
      unsigned int offsetX;
      unsigned int offsetY;
      unsigned int offsetZ;
      unsigned int extentWidth;
      unsigned int extentHeight;
      unsigned int extentDepth;
    } sparseLevel;
    struct {
      unsigned int layer;
      unsigned long long offset;
      unsigned long long size;
    } miptail;
  } subresource;
  CUmemOperationType memOperationType;
  CUmemHandleType memHandleType;
  union {
    CUmemGenericAllocationHandle memHandle;
  } memHandle;
  unsigned long long offset;
  unsigned int deviceBitMask;
  unsigned int flags;
  unsigned int reserved[2];
} CUarrayMapInfo_v1;

typedef CUarrayMapInfo_v1 CUarrayMapInfo;

typedef struct CUmemLocation_st {
  CUmemLocationType type;
  int id;
} CUmemLocation_v1;

typedef CUmemLocation_v1 CUmemLocation;

typedef enum CUmemAllocationCompType_enum {
  CU_MEM_ALLOCATION_COMP_NONE = 0x0,
  CU_MEM_ALLOCATION_COMP_GENERIC = 0x1,
} CUmemAllocationCompType;

typedef struct CUmemAllocationProp_st {
  CUmemAllocationType type;
  CUmemAllocationHandleType requestedHandleTypes;
  CUmemLocation location;
  void* win32HandleMetaData;
  struct {
    unsigned char compressionType;
    unsigned char gpuDirectRDMACapable;
    unsigned short usage;
    unsigned char reserved[4];
  } allocFlags;
} CUmemAllocationProp_v1;

typedef CUmemAllocationProp_v1 CUmemAllocationProp;

typedef enum CUmulticastGranularity_flags_enum {
  CU_MULTICAST_GRANULARITY_MINIMUM = 0x0,
  CU_MULTICAST_GRANULARITY_RECOMMENDED = 0x1,
} CUmulticastGranularity_flags;

typedef struct CUmulticastObjectProp_st {
  unsigned int numDevices;
  size_t size;
  unsigned long long handleTypes;
  unsigned long long flags;
} CUmulticastObjectProp_v1;

typedef CUmulticastObjectProp_v1 CUmulticastObjectProp;

typedef struct CUmemAccessDesc_st {
  CUmemLocation location;
  CUmemAccess_flags flags;
} CUmemAccessDesc_v1;

typedef CUmemAccessDesc_v1 CUmemAccessDesc;

typedef enum CUgraphExecUpdateResult_enum {
  CU_GRAPH_EXEC_UPDATE_SUCCESS = 0x0,
  CU_GRAPH_EXEC_UPDATE_ERROR = 0x1,
  CU_GRAPH_EXEC_UPDATE_ERROR_TOPOLOGY_CHANGED = 0x2,
  CU_GRAPH_EXEC_UPDATE_ERROR_NODE_TYPE_CHANGED = 0x3,
  CU_GRAPH_EXEC_UPDATE_ERROR_FUNCTION_CHANGED = 0x4,
  CU_GRAPH_EXEC_UPDATE_ERROR_PARAMETERS_CHANGED = 0x5,
  CU_GRAPH_EXEC_UPDATE_ERROR_NOT_SUPPORTED = 0x6,
  CU_GRAPH_EXEC_UPDATE_ERROR_UNSUPPORTED_FUNCTION_CHANGE = 0x7,
  CU_GRAPH_EXEC_UPDATE_ERROR_ATTRIBUTES_CHANGED = 0x8,
} CUgraphExecUpdateResult;

typedef struct CUgraphExecUpdateResultInfo_st {
  CUgraphExecUpdateResult result;
  CUgraphNode errorNode;
  CUgraphNode errorFromNode;
} CUgraphExecUpdateResultInfo_v1;

typedef CUgraphExecUpdateResultInfo_v1 CUgraphExecUpdateResultInfo;

typedef enum CUmemPool_attribute_enum {
  CU_MEMPOOL_ATTR_REUSE_FOLLOW_EVENT_DEPENDENCIES = 1,
  CU_MEMPOOL_ATTR_REUSE_ALLOW_OPPORTUNISTIC,
  CU_MEMPOOL_ATTR_REUSE_ALLOW_INTERNAL_DEPENDENCIES,
  CU_MEMPOOL_ATTR_RELEASE_THRESHOLD,
  CU_MEMPOOL_ATTR_RESERVED_MEM_CURRENT,
  CU_MEMPOOL_ATTR_RESERVED_MEM_HIGH,
  CU_MEMPOOL_ATTR_USED_MEM_CURRENT,
  CU_MEMPOOL_ATTR_USED_MEM_HIGH,
} CUmemPool_attribute;

typedef struct CUmemPoolProps_st {
  CUmemAllocationType allocType;
  CUmemAllocationHandleType handleTypes;
  CUmemLocation location;
  void* win32SecurityAttributes;
  unsigned char reserved[CU_IPC_HANDLE_SIZE];
} CUmemPoolProps_v1;

typedef CUmemPoolProps_v1 CUmemPoolProps;

typedef struct CUmemPoolPtrExportData_st {
  unsigned char reserved[CU_IPC_HANDLE_SIZE];
} CUmemPoolPtrExportData_v1;

typedef CUmemPoolPtrExportData_v1 CUmemPoolPtrExportData;

typedef struct CUDA_MEM_ALLOC_NODE_PARAMS_st {
  CUmemPoolProps poolProps;
  const CUmemAccessDesc* accessDescs;
  size_t accessDescCount;
  size_t bytesize;
  CUdeviceptr dptr;
} CUDA_MEM_ALLOC_NODE_PARAMS;

typedef enum CUgraphMem_attribute_enum {
  CU_GRAPH_MEM_ATTR_USED_MEM_CURRENT,
  CU_GRAPH_MEM_ATTR_USED_MEM_HIGH,
  CU_GRAPH_MEM_ATTR_RESERVED_MEM_CURRENT,
  CU_GRAPH_MEM_ATTR_RESERVED_MEM_HIGH,
} CUgraphMem_attribute;

typedef enum CUflushGPUDirectRDMAWritesOptions_enum {
  CU_FLUSH_GPU_DIRECT_RDMA_WRITES_OPTION_HOST = (1 << 0),
  CU_FLUSH_GPU_DIRECT_RDMA_WRITES_OPTION_MEMOPS = (1 << 1),
} CUflushGPUDirectRDMAWritesOptions;

typedef enum CUGPUDirectRDMAWritesOrdering_enum {
  CU_GPU_DIRECT_RDMA_WRITES_ORDERING_NONE = 0,
  CU_GPU_DIRECT_RDMA_WRITES_ORDERING_OWNER = 100,
  CU_GPU_DIRECT_RDMA_WRITES_ORDERING_ALL_DEVICES = 200,
} CUGPUDirectRDMAWritesOrdering;

typedef enum CUflushGPUDirectRDMAWritesScope_enum {
  CU_FLUSH_GPU_DIRECT_RDMA_WRITES_TO_OWNER = 100,
  CU_FLUSH_GPU_DIRECT_RDMA_WRITES_TO_ALL_DEVICES = 200,
} CUflushGPUDirectRDMAWritesScope;

typedef enum CUflushGPUDirectRDMAWritesTarget_enum {
  CU_FLUSH_GPU_DIRECT_RDMA_WRITES_TARGET_CURRENT_CTX = 0,
} CUflushGPUDirectRDMAWritesTarget;

typedef enum CUgraphDebugDot_flags_enum {
  CU_GRAPH_DEBUG_DOT_FLAGS_VERBOSE = (1 << 0),
  CU_GRAPH_DEBUG_DOT_FLAGS_RUNTIME_TYPES = (1 << 1),
  CU_GRAPH_DEBUG_DOT_FLAGS_KERNEL_NODE_PARAMS = (1 << 2),
  CU_GRAPH_DEBUG_DOT_FLAGS_MEMCPY_NODE_PARAMS = (1 << 3),
  CU_GRAPH_DEBUG_DOT_FLAGS_MEMSET_NODE_PARAMS = (1 << 4),
  CU_GRAPH_DEBUG_DOT_FLAGS_HOST_NODE_PARAMS = (1 << 5),
  CU_GRAPH_DEBUG_DOT_FLAGS_EVENT_NODE_PARAMS = (1 << 6),
  CU_GRAPH_DEBUG_DOT_FLAGS_EXT_SEMAS_SIGNAL_NODE_PARAMS = (1 << 7),
  CU_GRAPH_DEBUG_DOT_FLAGS_EXT_SEMAS_WAIT_NODE_PARAMS = (1 << 8),
  CU_GRAPH_DEBUG_DOT_FLAGS_KERNEL_NODE_ATTRIBUTES = (1 << 9),
  CU_GRAPH_DEBUG_DOT_FLAGS_HANDLES = (1 << 10),
  CU_GRAPH_DEBUG_DOT_FLAGS_MEM_ALLOC_NODE_PARAMS = (1 << 11),
  CU_GRAPH_DEBUG_DOT_FLAGS_MEM_FREE_NODE_PARAMS = (1 << 12),
  CU_GRAPH_DEBUG_DOT_FLAGS_BATCH_MEM_OP_NODE_PARAMS = (1 << 13),
  CU_GRAPH_DEBUG_DOT_FLAGS_EXTRA_TOPO_INFO = (1 << 14),
} CUgraphDebugDot_flags;

typedef enum CUuserObject_flags_enum {
  CU_USER_OBJECT_NO_DESTRUCTOR_SYNC = 1,
} CUuserObject_flags;

typedef enum CUuserObjectRetain_flags_enum {
  CU_GRAPH_USER_OBJECT_MOVE = 1,
} CUuserObjectRetain_flags;

typedef enum CUgraphInstantiate_flags_enum {
  CUDA_GRAPH_INSTANTIATE_FLAG_AUTO_FREE_ON_LAUNCH = 1,
  CUDA_GRAPH_INSTANTIATE_FLAG_UPLOAD = 2,
  CUDA_GRAPH_INSTANTIATE_FLAG_DEVICE_LAUNCH = 4,
  CUDA_GRAPH_INSTANTIATE_FLAG_USE_NODE_PRIORITY = 8,
} CUgraphInstantiate_flags;

typedef enum CUmoduleLoadingMode_enum {
  CU_MODULE_EAGER_LOADING = 0x1,
  CU_MODULE_LAZY_LOADING = 0x2,
} CUmoduleLoadingMode;

typedef enum CUcoredumpSettings_enum {
  CU_COREDUMP_ENABLE_ON_EXCEPTION = 1,
  CU_COREDUMP_TRIGGER_HOST,
  CU_COREDUMP_LIGHTWEIGHT,
  CU_COREDUMP_ENABLE_USER_TRIGGER,
  CU_COREDUMP_FILE,
  CU_COREDUMP_PIPE,
  CU_COREDUMP_MAX,
} CUcoredumpSettings;

typedef enum  {
  NVRTC_SUCCESS = 0,
  NVRTC_ERROR_OUT_OF_MEMORY = 1,
  NVRTC_ERROR_PROGRAM_CREATION_FAILURE = 2,
  NVRTC_ERROR_INVALID_INPUT = 3,
  NVRTC_ERROR_INVALID_PROGRAM = 4,
  NVRTC_ERROR_INVALID_OPTION = 5,
  NVRTC_ERROR_COMPILATION = 6,
  NVRTC_ERROR_BUILTIN_OPERATION_FAILURE = 7,
  NVRTC_ERROR_NO_NAME_EXPRESSIONS_AFTER_COMPILATION = 8,
  NVRTC_ERROR_NO_LOWERED_NAMES_BEFORE_COMPILATION = 9,
  NVRTC_ERROR_NAME_EXPRESSION_NOT_VALID = 10,
  NVRTC_ERROR_INTERNAL_ERROR = 11,
  NVRTC_ERROR_TIME_FILE_WRITE_FAILED = 12,
} nvrtcResult;

typedef struct _nvrtcProgram* nvrtcProgram;
typedef struct cudnnContext* cudnnHandle_t;

typedef enum  {
  CUDNN_STATUS_SUCCESS = 0,
  CUDNN_STATUS_NOT_INITIALIZED = 1,
  CUDNN_STATUS_ALLOC_FAILED = 2,
  CUDNN_STATUS_BAD_PARAM = 3,
  CUDNN_STATUS_INTERNAL_ERROR = 4,
  CUDNN_STATUS_INVALID_VALUE = 5,
  CUDNN_STATUS_ARCH_MISMATCH = 6,
  CUDNN_STATUS_MAPPING_ERROR = 7,
  CUDNN_STATUS_EXECUTION_FAILED = 8,
  CUDNN_STATUS_NOT_SUPPORTED = 9,
  CUDNN_STATUS_LICENSE_ERROR = 10,
  CUDNN_STATUS_RUNTIME_PREREQUISITE_MISSING = 11,
  CUDNN_STATUS_RUNTIME_IN_PROGRESS = 12,
  CUDNN_STATUS_RUNTIME_FP_OVERFLOW = 13,
  CUDNN_STATUS_VERSION_MISMATCH = 14,
} cudnnStatus_t;

typedef struct cudnnRuntimeTag_t cudnnRuntimeTag_t;

typedef enum  {
  CUDNN_ERRQUERY_RAWCODE = 0,
  CUDNN_ERRQUERY_NONBLOCKING = 1,
  CUDNN_ERRQUERY_BLOCKING = 2,
} cudnnErrQueryMode_t;

typedef enum libraryPropertyType_t {
  MAJOR_VERSION,
  MINOR_VERSION,
  PATCH_LEVEL,
} libraryPropertyType;

typedef struct cudnnTensorStruct* cudnnTensorDescriptor_t;
typedef struct cudnnPoolingStruct* cudnnPoolingDescriptor_t;
typedef struct cudnnFilterStruct* cudnnFilterDescriptor_t;
typedef struct cudnnLRNStruct* cudnnLRNDescriptor_t;
typedef struct cudnnActivationStruct* cudnnActivationDescriptor_t;
typedef struct cudnnSpatialTransformerStruct* cudnnSpatialTransformerDescriptor_t;
typedef struct cudnnOpTensorStruct* cudnnOpTensorDescriptor_t;
typedef struct cudnnReduceTensorStruct* cudnnReduceTensorDescriptor_t;
typedef struct cudnnCTCLossStruct* cudnnCTCLossDescriptor_t;
typedef struct cudnnTensorTransformStruct* cudnnTensorTransformDescriptor_t;

typedef enum  {
  CUDNN_DATA_FLOAT = 0,
  CUDNN_DATA_DOUBLE = 1,
  CUDNN_DATA_HALF = 2,
  CUDNN_DATA_INT8 = 3,
  CUDNN_DATA_INT32 = 4,
  CUDNN_DATA_INT8x4 = 5,
  CUDNN_DATA_UINT8 = 6,
  CUDNN_DATA_UINT8x4 = 7,
  CUDNN_DATA_INT8x32 = 8,
  CUDNN_DATA_BFLOAT16 = 9,
  CUDNN_DATA_INT64 = 10,
  CUDNN_DATA_BOOLEAN = 11,
  CUDNN_DATA_FP8_E4M3 = 12,
  CUDNN_DATA_FP8_E5M2 = 13,
  CUDNN_DATA_FAST_FLOAT_FOR_FP8 = 14,
} cudnnDataType_t;

typedef enum  {
  CUDNN_DEFAULT_MATH = 0,
  CUDNN_TENSOR_OP_MATH = 1,
  CUDNN_TENSOR_OP_MATH_ALLOW_CONVERSION = 2,
  CUDNN_FMA_MATH = 3,
} cudnnMathType_t;

typedef enum  {
  CUDNN_NOT_PROPAGATE_NAN = 0,
  CUDNN_PROPAGATE_NAN = 1,
} cudnnNanPropagation_t;

typedef enum  {
  CUDNN_NON_DETERMINISTIC = 0,
  CUDNN_DETERMINISTIC = 1,
} cudnnDeterminism_t;

typedef enum  {
  CUDNN_TENSOR_NCHW = 0,
  CUDNN_TENSOR_NHWC = 1,
  CUDNN_TENSOR_NCHW_VECT_C = 2,
} cudnnTensorFormat_t;

typedef enum  {
  CUDNN_TRANSFORM_FOLD = 0U,
  CUDNN_TRANSFORM_UNFOLD = 1U,
} cudnnFoldingDirection_t;

typedef enum  {
  CUDNN_OP_TENSOR_ADD = 0,
  CUDNN_OP_TENSOR_MUL = 1,
  CUDNN_OP_TENSOR_MIN = 2,
  CUDNN_OP_TENSOR_MAX = 3,
  CUDNN_OP_TENSOR_SQRT = 4,
  CUDNN_OP_TENSOR_NOT = 5,
} cudnnOpTensorOp_t;

typedef enum  {
  CUDNN_REDUCE_TENSOR_ADD = 0,
  CUDNN_REDUCE_TENSOR_MUL = 1,
  CUDNN_REDUCE_TENSOR_MIN = 2,
  CUDNN_REDUCE_TENSOR_MAX = 3,
  CUDNN_REDUCE_TENSOR_AMAX = 4,
  CUDNN_REDUCE_TENSOR_AVG = 5,
  CUDNN_REDUCE_TENSOR_NORM1 = 6,
  CUDNN_REDUCE_TENSOR_NORM2 = 7,
  CUDNN_REDUCE_TENSOR_MUL_NO_ZEROS = 8,
} cudnnReduceTensorOp_t;

typedef enum  {
  CUDNN_REDUCE_TENSOR_NO_INDICES = 0,
  CUDNN_REDUCE_TENSOR_FLATTENED_INDICES = 1,
} cudnnReduceTensorIndices_t;

typedef enum  {
  CUDNN_32BIT_INDICES = 0,
  CUDNN_64BIT_INDICES = 1,
  CUDNN_16BIT_INDICES = 2,
  CUDNN_8BIT_INDICES = 3,
} cudnnIndicesType_t;

typedef enum  {
  CUDNN_SOFTMAX_FAST = 0,
  CUDNN_SOFTMAX_ACCURATE = 1,
  CUDNN_SOFTMAX_LOG = 2,
} cudnnSoftmaxAlgorithm_t;

typedef enum  {
  CUDNN_SOFTMAX_MODE_INSTANCE = 0,
  CUDNN_SOFTMAX_MODE_CHANNEL = 1,
} cudnnSoftmaxMode_t;

typedef enum  {
  CUDNN_POOLING_MAX = 0,
  CUDNN_POOLING_AVERAGE_COUNT_INCLUDE_PADDING = 1,
  CUDNN_POOLING_AVERAGE_COUNT_EXCLUDE_PADDING = 2,
  CUDNN_POOLING_MAX_DETERMINISTIC = 3,
} cudnnPoolingMode_t;

typedef enum  {
  CUDNN_ACTIVATION_SIGMOID = 0,
  CUDNN_ACTIVATION_RELU = 1,
  CUDNN_ACTIVATION_TANH = 2,
  CUDNN_ACTIVATION_CLIPPED_RELU = 3,
  CUDNN_ACTIVATION_ELU = 4,
  CUDNN_ACTIVATION_IDENTITY = 5,
  CUDNN_ACTIVATION_SWISH = 6,
} cudnnActivationMode_t;

typedef enum  {
  CUDNN_LRN_CROSS_CHANNEL_DIM1 = 0,
} cudnnLRNMode_t;

typedef enum  {
  CUDNN_DIVNORM_PRECOMPUTED_MEANS = 0,
} cudnnDivNormMode_t;

typedef enum  {
  CUDNN_BATCHNORM_PER_ACTIVATION = 0,
  CUDNN_BATCHNORM_SPATIAL = 1,
  CUDNN_BATCHNORM_SPATIAL_PERSISTENT = 2,
} cudnnBatchNormMode_t;

typedef enum  {
  CUDNN_BATCHNORM_OPS_BN = 0,
  CUDNN_BATCHNORM_OPS_BN_ACTIVATION = 1,
  CUDNN_BATCHNORM_OPS_BN_ADD_ACTIVATION = 2,
} cudnnBatchNormOps_t;

typedef enum  {
  CUDNN_NORM_PER_ACTIVATION = 0,
  CUDNN_NORM_PER_CHANNEL = 1,
} cudnnNormMode_t;

typedef enum  {
  CUDNN_NORM_ALGO_STANDARD = 0,
  CUDNN_NORM_ALGO_PERSIST = 1,
} cudnnNormAlgo_t;

typedef enum  {
  CUDNN_NORM_OPS_NORM = 0,
  CUDNN_NORM_OPS_NORM_ACTIVATION = 1,
  CUDNN_NORM_OPS_NORM_ADD_ACTIVATION = 2,
} cudnnNormOps_t;

typedef enum  {
  CUDNN_SAMPLER_BILINEAR = 0,
} cudnnSamplerType_t;

typedef struct cudnnDropoutStruct* cudnnDropoutDescriptor_t;
typedef struct cudnnAlgorithmStruct* cudnnAlgorithmDescriptor_t;
typedef struct cudnnAlgorithmPerformanceStruct* cudnnAlgorithmPerformance_t;

typedef enum  {
  CUDNN_CONVOLUTION_FWD_ALGO_IMPLICIT_GEMM = 0,
  CUDNN_CONVOLUTION_FWD_ALGO_IMPLICIT_PRECOMP_GEMM = 1,
  CUDNN_CONVOLUTION_FWD_ALGO_GEMM = 2,
  CUDNN_CONVOLUTION_FWD_ALGO_DIRECT = 3,
  CUDNN_CONVOLUTION_FWD_ALGO_FFT = 4,
  CUDNN_CONVOLUTION_FWD_ALGO_FFT_TILING = 5,
  CUDNN_CONVOLUTION_FWD_ALGO_WINOGRAD = 6,
  CUDNN_CONVOLUTION_FWD_ALGO_WINOGRAD_NONFUSED = 7,
  CUDNN_CONVOLUTION_FWD_ALGO_COUNT = 8,
} cudnnConvolutionFwdAlgo_t;

typedef enum  {
  CUDNN_CONVOLUTION_BWD_FILTER_ALGO_0 = 0,
  CUDNN_CONVOLUTION_BWD_FILTER_ALGO_1 = 1,
  CUDNN_CONVOLUTION_BWD_FILTER_ALGO_FFT = 2,
  CUDNN_CONVOLUTION_BWD_FILTER_ALGO_3 = 3,
  CUDNN_CONVOLUTION_BWD_FILTER_ALGO_WINOGRAD = 4,
  CUDNN_CONVOLUTION_BWD_FILTER_ALGO_WINOGRAD_NONFUSED = 5,
  CUDNN_CONVOLUTION_BWD_FILTER_ALGO_FFT_TILING = 6,
  CUDNN_CONVOLUTION_BWD_FILTER_ALGO_COUNT = 7,
} cudnnConvolutionBwdFilterAlgo_t;

typedef enum  {
  CUDNN_CONVOLUTION_BWD_DATA_ALGO_0 = 0,
  CUDNN_CONVOLUTION_BWD_DATA_ALGO_1 = 1,
  CUDNN_CONVOLUTION_BWD_DATA_ALGO_FFT = 2,
  CUDNN_CONVOLUTION_BWD_DATA_ALGO_FFT_TILING = 3,
  CUDNN_CONVOLUTION_BWD_DATA_ALGO_WINOGRAD = 4,
  CUDNN_CONVOLUTION_BWD_DATA_ALGO_WINOGRAD_NONFUSED = 5,
  CUDNN_CONVOLUTION_BWD_DATA_ALGO_COUNT = 6,
} cudnnConvolutionBwdDataAlgo_t;

typedef enum  {
  CUDNN_RNN_ALGO_STANDARD = 0,
  CUDNN_RNN_ALGO_PERSIST_STATIC = 1,
  CUDNN_RNN_ALGO_PERSIST_DYNAMIC = 2,
  CUDNN_RNN_ALGO_PERSIST_STATIC_SMALL_H = 3,
  CUDNN_RNN_ALGO_COUNT = 4,
} cudnnRNNAlgo_t;

typedef enum  {
  CUDNN_CTC_LOSS_ALGO_DETERMINISTIC = 0,
  CUDNN_CTC_LOSS_ALGO_NON_DETERMINISTIC = 1,
} cudnnCTCLossAlgo_t;

typedef struct cudnnAlgorithmUnionStruct {
  union Algorithm {
    cudnnConvolutionFwdAlgo_t convFwdAlgo;
    cudnnConvolutionBwdFilterAlgo_t convBwdFilterAlgo;
    cudnnConvolutionBwdDataAlgo_t convBwdDataAlgo;
    cudnnRNNAlgo_t RNNAlgo;
    cudnnCTCLossAlgo_t CTCLossAlgo;
  } algo;
} cudnnAlgorithm_t;

typedef enum  {
  CUDNN_SEV_FATAL = 0,
  CUDNN_SEV_ERROR = 1,
  CUDNN_SEV_WARNING = 2,
  CUDNN_SEV_INFO = 3,
} cudnnSeverity_t;

typedef struct cudnnDebugStruct {
  unsigned cudnn_version;
  cudnnStatus_t cudnnStatus;
  unsigned time_sec;
  unsigned time_usec;
  unsigned time_delta;
  cudnnHandle_t handle;
  cudaStream_t stream;
  unsigned long long pid;
  unsigned long long tid;
  int cudaDeviceId;
  int reserved[15];
} cudnnDebug_t;

typedef void (CUDA_CB *cudnnCallback_t)(cudnnSeverity_t sev, void* udata, const cudnnDebug_t* dbg, const char* msg);

typedef enum  {
  CUDNN_FWD_MODE_INFERENCE = 0,
  CUDNN_FWD_MODE_TRAINING = 1,
} cudnnForwardMode_t;

typedef enum  {
  CUDNN_RNN_RELU = 0,
  CUDNN_RNN_TANH = 1,
  CUDNN_LSTM = 2,
  CUDNN_GRU = 3,
} cudnnRNNMode_t;

typedef enum  {
  CUDNN_RNN_NO_BIAS = 0,
  CUDNN_RNN_SINGLE_INP_BIAS = 1,
  CUDNN_RNN_DOUBLE_BIAS = 2,
  CUDNN_RNN_SINGLE_REC_BIAS = 3,
} cudnnRNNBiasMode_t;

typedef enum  {
  CUDNN_UNIDIRECTIONAL = 0,
  CUDNN_BIDIRECTIONAL = 1,
} cudnnDirectionMode_t;

typedef enum  {
  CUDNN_LINEAR_INPUT = 0,
  CUDNN_SKIP_INPUT = 1,
} cudnnRNNInputMode_t;

typedef enum  {
  CUDNN_RNN_CLIP_NONE = 0,
  CUDNN_RNN_CLIP_MINMAX = 1,
} cudnnRNNClipMode_t;

typedef enum  {
  CUDNN_RNN_DATA_LAYOUT_SEQ_MAJOR_UNPACKED = 0,
  CUDNN_RNN_DATA_LAYOUT_SEQ_MAJOR_PACKED = 1,
  CUDNN_RNN_DATA_LAYOUT_BATCH_MAJOR_UNPACKED = 2,
} cudnnRNNDataLayout_t;

typedef unsigned cudnnRNNPaddingMode_t;
typedef struct cudnnRNNStruct* cudnnRNNDescriptor_t;
typedef struct cudnnPersistentRNNPlan* cudnnPersistentRNNPlan_t;
typedef struct cudnnRNNDataStruct* cudnnRNNDataDescriptor_t;

typedef enum  {
  CUDNN_SEQDATA_TIME_DIM = 0,
  CUDNN_SEQDATA_BATCH_DIM = 1,
  CUDNN_SEQDATA_BEAM_DIM = 2,
  CUDNN_SEQDATA_VECT_DIM = 3,
} cudnnSeqDataAxis_t;

typedef struct cudnnSeqDataStruct* cudnnSeqDataDescriptor_t;
typedef unsigned cudnnAttnQueryMap_t;
typedef struct cudnnAttnStruct* cudnnAttnDescriptor_t;

typedef enum  {
  CUDNN_MH_ATTN_Q_WEIGHTS = 0,
  CUDNN_MH_ATTN_K_WEIGHTS = 1,
  CUDNN_MH_ATTN_V_WEIGHTS = 2,
  CUDNN_MH_ATTN_O_WEIGHTS = 3,
  CUDNN_MH_ATTN_Q_BIASES = 4,
  CUDNN_MH_ATTN_K_BIASES = 5,
  CUDNN_MH_ATTN_V_BIASES = 6,
  CUDNN_MH_ATTN_O_BIASES = 7,
} cudnnMultiHeadAttnWeightKind_t;

typedef enum  {
  CUDNN_WGRAD_MODE_ADD = 0,
  CUDNN_WGRAD_MODE_SET = 1,
} cudnnWgradMode_t;

typedef enum  {
  CUDNN_LOSS_NORMALIZATION_NONE = 0,
  CUDNN_LOSS_NORMALIZATION_SOFTMAX = 1,
} cudnnLossNormalizationMode_t;

typedef struct cudnnConvolutionStruct* cudnnConvolutionDescriptor_t;

typedef enum  {
  CUDNN_CONVOLUTION = 0,
  CUDNN_CROSS_CORRELATION = 1,
} cudnnConvolutionMode_t;

typedef enum  {
  CUDNN_DEFAULT_REORDER = 0,
  CUDNN_NO_REORDER = 1,
} cudnnReorderType_t;

typedef struct cudnnConvolutionFwdAlgoPerfStruct {
  cudnnConvolutionFwdAlgo_t algo;
  cudnnStatus_t status;
  float time;
  size_t memory;
  cudnnDeterminism_t determinism;
  cudnnMathType_t mathType;
  int reserved[3];
} cudnnConvolutionFwdAlgoPerf_t;

typedef struct cudnnConvolutionBwdDataAlgoPerfStruct {
  cudnnConvolutionBwdDataAlgo_t algo;
  cudnnStatus_t status;
  float time;
  size_t memory;
  cudnnDeterminism_t determinism;
  cudnnMathType_t mathType;
  int reserved[3];
} cudnnConvolutionBwdDataAlgoPerf_t;

typedef struct cudnnFusedOpsConstParamStruct* cudnnFusedOpsConstParamPack_t;
typedef struct cudnnFusedOpsVariantParamStruct* cudnnFusedOpsVariantParamPack_t;
typedef struct cudnnFusedOpsPlanStruct* cudnnFusedOpsPlan_t;

typedef enum  {
  CUDNN_FUSED_SCALE_BIAS_ACTIVATION_CONV_BNSTATS = 0,
  CUDNN_FUSED_SCALE_BIAS_ACTIVATION_WGRAD = 1,
  CUDNN_FUSED_BN_FINALIZE_STATISTICS_TRAINING = 2,
  CUDNN_FUSED_BN_FINALIZE_STATISTICS_INFERENCE = 3,
  CUDNN_FUSED_CONV_SCALE_BIAS_ADD_ACTIVATION = 4,
  CUDNN_FUSED_SCALE_BIAS_ADD_ACTIVATION_GEN_BITMASK = 5,
  CUDNN_FUSED_DACTIVATION_FORK_DBATCHNORM = 6,
} cudnnFusedOps_t;

typedef enum  {
  CUDNN_PARAM_XDESC = 0,
  CUDNN_PARAM_XDATA_PLACEHOLDER = 1,
  CUDNN_PARAM_BN_MODE = 2,
  CUDNN_PARAM_BN_EQSCALEBIAS_DESC = 3,
  CUDNN_PARAM_BN_EQSCALE_PLACEHOLDER = 4,
  CUDNN_PARAM_BN_EQBIAS_PLACEHOLDER = 5,
  CUDNN_PARAM_ACTIVATION_DESC = 6,
  CUDNN_PARAM_CONV_DESC = 7,
  CUDNN_PARAM_WDESC = 8,
  CUDNN_PARAM_WDATA_PLACEHOLDER = 9,
  CUDNN_PARAM_DWDESC = 10,
  CUDNN_PARAM_DWDATA_PLACEHOLDER = 11,
  CUDNN_PARAM_YDESC = 12,
  CUDNN_PARAM_YDATA_PLACEHOLDER = 13,
  CUDNN_PARAM_DYDESC = 14,
  CUDNN_PARAM_DYDATA_PLACEHOLDER = 15,
  CUDNN_PARAM_YSTATS_DESC = 16,
  CUDNN_PARAM_YSUM_PLACEHOLDER = 17,
  CUDNN_PARAM_YSQSUM_PLACEHOLDER = 18,
  CUDNN_PARAM_BN_SCALEBIAS_MEANVAR_DESC = 19,
  CUDNN_PARAM_BN_SCALE_PLACEHOLDER = 20,
  CUDNN_PARAM_BN_BIAS_PLACEHOLDER = 21,
  CUDNN_PARAM_BN_SAVED_MEAN_PLACEHOLDER = 22,
  CUDNN_PARAM_BN_SAVED_INVSTD_PLACEHOLDER = 23,
  CUDNN_PARAM_BN_RUNNING_MEAN_PLACEHOLDER = 24,
  CUDNN_PARAM_BN_RUNNING_VAR_PLACEHOLDER = 25,
  CUDNN_PARAM_ZDESC = 26,
  CUDNN_PARAM_ZDATA_PLACEHOLDER = 27,
  CUDNN_PARAM_BN_Z_EQSCALEBIAS_DESC = 28,
  CUDNN_PARAM_BN_Z_EQSCALE_PLACEHOLDER = 29,
  CUDNN_PARAM_BN_Z_EQBIAS_PLACEHOLDER = 30,
  CUDNN_PARAM_ACTIVATION_BITMASK_DESC = 31,
  CUDNN_PARAM_ACTIVATION_BITMASK_PLACEHOLDER = 32,
  CUDNN_PARAM_DXDESC = 33,
  CUDNN_PARAM_DXDATA_PLACEHOLDER = 34,
  CUDNN_PARAM_DZDESC = 35,
  CUDNN_PARAM_DZDATA_PLACEHOLDER = 36,
  CUDNN_PARAM_BN_DSCALE_PLACEHOLDER = 37,
  CUDNN_PARAM_BN_DBIAS_PLACEHOLDER = 38,
} cudnnFusedOpsConstParamLabel_t;

typedef enum  {
  CUDNN_PTR_NULL = 0,
  CUDNN_PTR_ELEM_ALIGNED = 1,
  CUDNN_PTR_16B_ALIGNED = 2,
} cudnnFusedOpsPointerPlaceHolder_t;

typedef enum  {
  CUDNN_PTR_XDATA = 0,
  CUDNN_PTR_BN_EQSCALE = 1,
  CUDNN_PTR_BN_EQBIAS = 2,
  CUDNN_PTR_WDATA = 3,
  CUDNN_PTR_DWDATA = 4,
  CUDNN_PTR_YDATA = 5,
  CUDNN_PTR_DYDATA = 6,
  CUDNN_PTR_YSUM = 7,
  CUDNN_PTR_YSQSUM = 8,
  CUDNN_PTR_WORKSPACE = 9,
  CUDNN_PTR_BN_SCALE = 10,
  CUDNN_PTR_BN_BIAS = 11,
  CUDNN_PTR_BN_SAVED_MEAN = 12,
  CUDNN_PTR_BN_SAVED_INVSTD = 13,
  CUDNN_PTR_BN_RUNNING_MEAN = 14,
  CUDNN_PTR_BN_RUNNING_VAR = 15,
  CUDNN_PTR_ZDATA = 16,
  CUDNN_PTR_BN_Z_EQSCALE = 17,
  CUDNN_PTR_BN_Z_EQBIAS = 18,
  CUDNN_PTR_ACTIVATION_BITMASK = 19,
  CUDNN_PTR_DXDATA = 20,
  CUDNN_PTR_DZDATA = 21,
  CUDNN_PTR_BN_DSCALE = 22,
  CUDNN_PTR_BN_DBIAS = 23,
  CUDNN_SCALAR_SIZE_T_WORKSPACE_SIZE_IN_BYTES = 100,
  CUDNN_SCALAR_INT64_T_BN_ACCUMULATION_COUNT = 101,
  CUDNN_SCALAR_DOUBLE_BN_EXP_AVG_FACTOR = 102,
  CUDNN_SCALAR_DOUBLE_BN_EPSILON = 103,
} cudnnFusedOpsVariantParamLabel_t;

typedef struct cudnnConvolutionBwdFilterAlgoPerfStruct {
  cudnnConvolutionBwdFilterAlgo_t algo;
  cudnnStatus_t status;
  float time;
  size_t memory;
  cudnnDeterminism_t determinism;
  cudnnMathType_t mathType;
  int reserved[3];
} cudnnConvolutionBwdFilterAlgoPerf_t;

typedef void* cudnnBackendDescriptor_t;

typedef struct cudnnFractionStruct {
  int64_t numerator;
  int64_t denominator;
} cudnnFraction_t;

typedef enum  {
  CUDNN_POINTWISE_ADD = 0,
  CUDNN_POINTWISE_ADD_SQUARE = 5,
  CUDNN_POINTWISE_DIV = 6,
  CUDNN_POINTWISE_MAX = 3,
  CUDNN_POINTWISE_MIN = 2,
  CUDNN_POINTWISE_MOD = 7,
  CUDNN_POINTWISE_MUL = 1,
  CUDNN_POINTWISE_POW = 8,
  CUDNN_POINTWISE_SUB = 9,
  CUDNN_POINTWISE_ABS = 10,
  CUDNN_POINTWISE_CEIL = 11,
  CUDNN_POINTWISE_COS = 12,
  CUDNN_POINTWISE_EXP = 13,
  CUDNN_POINTWISE_FLOOR = 14,
  CUDNN_POINTWISE_LOG = 15,
  CUDNN_POINTWISE_NEG = 16,
  CUDNN_POINTWISE_RSQRT = 17,
  CUDNN_POINTWISE_SIN = 18,
  CUDNN_POINTWISE_SQRT = 4,
  CUDNN_POINTWISE_TAN = 19,
  CUDNN_POINTWISE_ERF = 20,
  CUDNN_POINTWISE_IDENTITY = 21,
  CUDNN_POINTWISE_RECIPROCAL = 22,
  CUDNN_POINTWISE_RELU_FWD = 100,
  CUDNN_POINTWISE_TANH_FWD = 101,
  CUDNN_POINTWISE_SIGMOID_FWD = 102,
  CUDNN_POINTWISE_ELU_FWD = 103,
  CUDNN_POINTWISE_GELU_FWD = 104,
  CUDNN_POINTWISE_SOFTPLUS_FWD = 105,
  CUDNN_POINTWISE_SWISH_FWD = 106,
  CUDNN_POINTWISE_GELU_APPROX_TANH_FWD = 107,
  CUDNN_POINTWISE_RELU_BWD = 200,
  CUDNN_POINTWISE_TANH_BWD = 201,
  CUDNN_POINTWISE_SIGMOID_BWD = 202,
  CUDNN_POINTWISE_ELU_BWD = 203,
  CUDNN_POINTWISE_GELU_BWD = 204,
  CUDNN_POINTWISE_SOFTPLUS_BWD = 205,
  CUDNN_POINTWISE_SWISH_BWD = 206,
  CUDNN_POINTWISE_GELU_APPROX_TANH_BWD = 207,
  CUDNN_POINTWISE_CMP_EQ = 300,
  CUDNN_POINTWISE_CMP_NEQ = 301,
  CUDNN_POINTWISE_CMP_GT = 302,
  CUDNN_POINTWISE_CMP_GE = 303,
  CUDNN_POINTWISE_CMP_LT = 304,
  CUDNN_POINTWISE_CMP_LE = 305,
  CUDNN_POINTWISE_LOGICAL_AND = 400,
  CUDNN_POINTWISE_LOGICAL_OR = 401,
  CUDNN_POINTWISE_LOGICAL_NOT = 402,
  CUDNN_POINTWISE_GEN_INDEX = 501,
  CUDNN_POINTWISE_BINARY_SELECT = 601,
} cudnnPointwiseMode_t;

typedef enum  {
  CUDNN_RESAMPLE_NEAREST = 0,
  CUDNN_RESAMPLE_BILINEAR = 1,
  CUDNN_RESAMPLE_AVGPOOL = 2,
  CUDNN_RESAMPLE_AVGPOOL_INCLUDE_PADDING = 2,
  CUDNN_RESAMPLE_AVGPOOL_EXCLUDE_PADDING = 4,
  CUDNN_RESAMPLE_MAXPOOL = 3,
} cudnnResampleMode_t;

typedef enum  {
  CUDNN_SIGNAL_SET = 0,
  CUDNN_SIGNAL_WAIT = 1,
} cudnnSignalMode_t;

typedef enum  {
  CUDNN_GENSTATS_SUM_SQSUM = 0,
} cudnnGenStatsMode_t;

typedef enum  {
  CUDNN_BN_FINALIZE_STATISTICS_TRAINING = 0,
  CUDNN_BN_FINALIZE_STATISTICS_INFERENCE = 1,
} cudnnBnFinalizeStatsMode_t;

typedef enum  {
  CUDNN_RNG_DISTRIBUTION_BERNOULLI,
  CUDNN_RNG_DISTRIBUTION_UNIFORM,
  CUDNN_RNG_DISTRIBUTION_NORMAL,
} cudnnRngDistribution_t;

typedef enum  {
  CUDNN_ATTR_POINTWISE_MODE = 0,
  CUDNN_ATTR_POINTWISE_MATH_PREC = 1,
  CUDNN_ATTR_POINTWISE_NAN_PROPAGATION = 2,
  CUDNN_ATTR_POINTWISE_RELU_LOWER_CLIP = 3,
  CUDNN_ATTR_POINTWISE_RELU_UPPER_CLIP = 4,
  CUDNN_ATTR_POINTWISE_RELU_LOWER_CLIP_SLOPE = 5,
  CUDNN_ATTR_POINTWISE_ELU_ALPHA = 6,
  CUDNN_ATTR_POINTWISE_SOFTPLUS_BETA = 7,
  CUDNN_ATTR_POINTWISE_SWISH_BETA = 8,
  CUDNN_ATTR_POINTWISE_AXIS = 9,
  CUDNN_ATTR_CONVOLUTION_COMP_TYPE = 100,
  CUDNN_ATTR_CONVOLUTION_CONV_MODE = 101,
  CUDNN_ATTR_CONVOLUTION_DILATIONS = 102,
  CUDNN_ATTR_CONVOLUTION_FILTER_STRIDES = 103,
  CUDNN_ATTR_CONVOLUTION_POST_PADDINGS = 104,
  CUDNN_ATTR_CONVOLUTION_PRE_PADDINGS = 105,
  CUDNN_ATTR_CONVOLUTION_SPATIAL_DIMS = 106,
  CUDNN_ATTR_ENGINEHEUR_MODE = 200,
  CUDNN_ATTR_ENGINEHEUR_OPERATION_GRAPH = 201,
  CUDNN_ATTR_ENGINEHEUR_RESULTS = 202,
  CUDNN_ATTR_ENGINECFG_ENGINE = 300,
  CUDNN_ATTR_ENGINECFG_INTERMEDIATE_INFO = 301,
  CUDNN_ATTR_ENGINECFG_KNOB_CHOICES = 302,
  CUDNN_ATTR_EXECUTION_PLAN_HANDLE = 400,
  CUDNN_ATTR_EXECUTION_PLAN_ENGINE_CONFIG = 401,
  CUDNN_ATTR_EXECUTION_PLAN_WORKSPACE_SIZE = 402,
  CUDNN_ATTR_EXECUTION_PLAN_COMPUTED_INTERMEDIATE_UIDS = 403,
  CUDNN_ATTR_EXECUTION_PLAN_RUN_ONLY_INTERMEDIATE_UIDS = 404,
  CUDNN_ATTR_EXECUTION_PLAN_JSON_REPRESENTATION = 405,
  CUDNN_ATTR_INTERMEDIATE_INFO_UNIQUE_ID = 500,
  CUDNN_ATTR_INTERMEDIATE_INFO_SIZE = 501,
  CUDNN_ATTR_INTERMEDIATE_INFO_DEPENDENT_DATA_UIDS = 502,
  CUDNN_ATTR_INTERMEDIATE_INFO_DEPENDENT_ATTRIBUTES = 503,
  CUDNN_ATTR_KNOB_CHOICE_KNOB_TYPE = 600,
  CUDNN_ATTR_KNOB_CHOICE_KNOB_VALUE = 601,
  CUDNN_ATTR_OPERATION_CONVOLUTION_FORWARD_ALPHA = 700,
  CUDNN_ATTR_OPERATION_CONVOLUTION_FORWARD_BETA = 701,
  CUDNN_ATTR_OPERATION_CONVOLUTION_FORWARD_CONV_DESC = 702,
  CUDNN_ATTR_OPERATION_CONVOLUTION_FORWARD_W = 703,
  CUDNN_ATTR_OPERATION_CONVOLUTION_FORWARD_X = 704,
  CUDNN_ATTR_OPERATION_CONVOLUTION_FORWARD_Y = 705,
  CUDNN_ATTR_OPERATION_CONVOLUTION_BWD_DATA_ALPHA = 706,
  CUDNN_ATTR_OPERATION_CONVOLUTION_BWD_DATA_BETA = 707,
  CUDNN_ATTR_OPERATION_CONVOLUTION_BWD_DATA_CONV_DESC = 708,
  CUDNN_ATTR_OPERATION_CONVOLUTION_BWD_DATA_W = 709,
  CUDNN_ATTR_OPERATION_CONVOLUTION_BWD_DATA_DX = 710,
  CUDNN_ATTR_OPERATION_CONVOLUTION_BWD_DATA_DY = 711,
  CUDNN_ATTR_OPERATION_CONVOLUTION_BWD_FILTER_ALPHA = 712,
  CUDNN_ATTR_OPERATION_CONVOLUTION_BWD_FILTER_BETA = 713,
  CUDNN_ATTR_OPERATION_CONVOLUTION_BWD_FILTER_CONV_DESC = 714,
  CUDNN_ATTR_OPERATION_CONVOLUTION_BWD_FILTER_DW = 715,
  CUDNN_ATTR_OPERATION_CONVOLUTION_BWD_FILTER_X = 716,
  CUDNN_ATTR_OPERATION_CONVOLUTION_BWD_FILTER_DY = 717,
  CUDNN_ATTR_OPERATION_POINTWISE_PW_DESCRIPTOR = 750,
  CUDNN_ATTR_OPERATION_POINTWISE_XDESC = 751,
  CUDNN_ATTR_OPERATION_POINTWISE_BDESC = 752,
  CUDNN_ATTR_OPERATION_POINTWISE_YDESC = 753,
  CUDNN_ATTR_OPERATION_POINTWISE_ALPHA1 = 754,
  CUDNN_ATTR_OPERATION_POINTWISE_ALPHA2 = 755,
  CUDNN_ATTR_OPERATION_POINTWISE_DXDESC = 756,
  CUDNN_ATTR_OPERATION_POINTWISE_DYDESC = 757,
  CUDNN_ATTR_OPERATION_POINTWISE_TDESC = 758,
  CUDNN_ATTR_OPERATION_GENSTATS_MODE = 770,
  CUDNN_ATTR_OPERATION_GENSTATS_MATH_PREC = 771,
  CUDNN_ATTR_OPERATION_GENSTATS_XDESC = 772,
  CUDNN_ATTR_OPERATION_GENSTATS_SUMDESC = 773,
  CUDNN_ATTR_OPERATION_GENSTATS_SQSUMDESC = 774,
  CUDNN_ATTR_OPERATION_BN_FINALIZE_STATS_MODE = 780,
  CUDNN_ATTR_OPERATION_BN_FINALIZE_MATH_PREC = 781,
  CUDNN_ATTR_OPERATION_BN_FINALIZE_Y_SUM_DESC = 782,
  CUDNN_ATTR_OPERATION_BN_FINALIZE_Y_SQ_SUM_DESC = 783,
  CUDNN_ATTR_OPERATION_BN_FINALIZE_SCALE_DESC = 784,
  CUDNN_ATTR_OPERATION_BN_FINALIZE_BIAS_DESC = 785,
  CUDNN_ATTR_OPERATION_BN_FINALIZE_PREV_RUNNING_MEAN_DESC = 786,
  CUDNN_ATTR_OPERATION_BN_FINALIZE_PREV_RUNNING_VAR_DESC = 787,
  CUDNN_ATTR_OPERATION_BN_FINALIZE_UPDATED_RUNNING_MEAN_DESC = 788,
  CUDNN_ATTR_OPERATION_BN_FINALIZE_UPDATED_RUNNING_VAR_DESC = 789,
  CUDNN_ATTR_OPERATION_BN_FINALIZE_SAVED_MEAN_DESC = 790,
  CUDNN_ATTR_OPERATION_BN_FINALIZE_SAVED_INV_STD_DESC = 791,
  CUDNN_ATTR_OPERATION_BN_FINALIZE_EQ_SCALE_DESC = 792,
  CUDNN_ATTR_OPERATION_BN_FINALIZE_EQ_BIAS_DESC = 793,
  CUDNN_ATTR_OPERATION_BN_FINALIZE_ACCUM_COUNT_DESC = 794,
  CUDNN_ATTR_OPERATION_BN_FINALIZE_EPSILON_DESC = 795,
  CUDNN_ATTR_OPERATION_BN_FINALIZE_EXP_AVERATE_FACTOR_DESC = 796,
  CUDNN_ATTR_OPERATIONGRAPH_HANDLE = 800,
  CUDNN_ATTR_OPERATIONGRAPH_OPS = 801,
  CUDNN_ATTR_OPERATIONGRAPH_ENGINE_GLOBAL_COUNT = 802,
  CUDNN_ATTR_TENSOR_BYTE_ALIGNMENT = 900,
  CUDNN_ATTR_TENSOR_DATA_TYPE = 901,
  CUDNN_ATTR_TENSOR_DIMENSIONS = 902,
  CUDNN_ATTR_TENSOR_STRIDES = 903,
  CUDNN_ATTR_TENSOR_VECTOR_COUNT = 904,
  CUDNN_ATTR_TENSOR_VECTORIZED_DIMENSION = 905,
  CUDNN_ATTR_TENSOR_UNIQUE_ID = 906,
  CUDNN_ATTR_TENSOR_IS_VIRTUAL = 907,
  CUDNN_ATTR_TENSOR_IS_BY_VALUE = 908,
  CUDNN_ATTR_TENSOR_REORDERING_MODE = 909,
  CUDNN_ATTR_TENSOR_RAGGED_OFFSET_DESC = 913,
  CUDNN_ATTR_VARIANT_PACK_UNIQUE_IDS = 1000,
  CUDNN_ATTR_VARIANT_PACK_DATA_POINTERS = 1001,
  CUDNN_ATTR_VARIANT_PACK_INTERMEDIATES = 1002,
  CUDNN_ATTR_VARIANT_PACK_WORKSPACE = 1003,
  CUDNN_ATTR_LAYOUT_INFO_TENSOR_UID = 1100,
  CUDNN_ATTR_LAYOUT_INFO_TYPES = 1101,
  CUDNN_ATTR_KNOB_INFO_TYPE = 1200,
  CUDNN_ATTR_KNOB_INFO_MAXIMUM_VALUE = 1201,
  CUDNN_ATTR_KNOB_INFO_MINIMUM_VALUE = 1202,
  CUDNN_ATTR_KNOB_INFO_STRIDE = 1203,
  CUDNN_ATTR_ENGINE_OPERATION_GRAPH = 1300,
  CUDNN_ATTR_ENGINE_GLOBAL_INDEX = 1301,
  CUDNN_ATTR_ENGINE_KNOB_INFO = 1302,
  CUDNN_ATTR_ENGINE_NUMERICAL_NOTE = 1303,
  CUDNN_ATTR_ENGINE_LAYOUT_INFO = 1304,
  CUDNN_ATTR_ENGINE_BEHAVIOR_NOTE = 1305,
  CUDNN_ATTR_MATMUL_COMP_TYPE = 1500,
  CUDNN_ATTR_MATMUL_PADDING_VALUE = 1503,
  CUDNN_ATTR_OPERATION_MATMUL_ADESC = 1520,
  CUDNN_ATTR_OPERATION_MATMUL_BDESC = 1521,
  CUDNN_ATTR_OPERATION_MATMUL_CDESC = 1522,
  CUDNN_ATTR_OPERATION_MATMUL_DESC = 1523,
  CUDNN_ATTR_OPERATION_MATMUL_IRREGULARLY_STRIDED_BATCH_COUNT = 1524,
  CUDNN_ATTR_OPERATION_MATMUL_GEMM_M_OVERRIDE_DESC = 1525,
  CUDNN_ATTR_OPERATION_MATMUL_GEMM_N_OVERRIDE_DESC = 1526,
  CUDNN_ATTR_OPERATION_MATMUL_GEMM_K_OVERRIDE_DESC = 1527,
  CUDNN_ATTR_REDUCTION_OPERATOR = 1600,
  CUDNN_ATTR_REDUCTION_COMP_TYPE = 1601,
  CUDNN_ATTR_OPERATION_REDUCTION_XDESC = 1610,
  CUDNN_ATTR_OPERATION_REDUCTION_YDESC = 1611,
  CUDNN_ATTR_OPERATION_REDUCTION_DESC = 1612,
  CUDNN_ATTR_OPERATION_BN_BWD_WEIGHTS_MATH_PREC = 1620,
  CUDNN_ATTR_OPERATION_BN_BWD_WEIGHTS_MEAN_DESC = 1621,
  CUDNN_ATTR_OPERATION_BN_BWD_WEIGHTS_INVSTD_DESC = 1622,
  CUDNN_ATTR_OPERATION_BN_BWD_WEIGHTS_BN_SCALE_DESC = 1623,
  CUDNN_ATTR_OPERATION_BN_BWD_WEIGHTS_X_DESC = 1624,
  CUDNN_ATTR_OPERATION_BN_BWD_WEIGHTS_DY_DESC = 1625,
  CUDNN_ATTR_OPERATION_BN_BWD_WEIGHTS_DBN_SCALE_DESC = 1626,
  CUDNN_ATTR_OPERATION_BN_BWD_WEIGHTS_DBN_BIAS_DESC = 1627,
  CUDNN_ATTR_OPERATION_BN_BWD_WEIGHTS_EQ_DY_SCALE_DESC = 1628,
  CUDNN_ATTR_OPERATION_BN_BWD_WEIGHTS_EQ_X_SCALE_DESC = 1629,
  CUDNN_ATTR_OPERATION_BN_BWD_WEIGHTS_EQ_BIAS = 1630,
  CUDNN_ATTR_RESAMPLE_MODE = 1700,
  CUDNN_ATTR_RESAMPLE_COMP_TYPE = 1701,
  CUDNN_ATTR_RESAMPLE_SPATIAL_DIMS = 1702,
  CUDNN_ATTR_RESAMPLE_POST_PADDINGS = 1703,
  CUDNN_ATTR_RESAMPLE_PRE_PADDINGS = 1704,
  CUDNN_ATTR_RESAMPLE_STRIDES = 1705,
  CUDNN_ATTR_RESAMPLE_WINDOW_DIMS = 1706,
  CUDNN_ATTR_RESAMPLE_NAN_PROPAGATION = 1707,
  CUDNN_ATTR_RESAMPLE_PADDING_MODE = 1708,
  CUDNN_ATTR_OPERATION_RESAMPLE_FWD_XDESC = 1710,
  CUDNN_ATTR_OPERATION_RESAMPLE_FWD_YDESC = 1711,
  CUDNN_ATTR_OPERATION_RESAMPLE_FWD_IDXDESC = 1712,
  CUDNN_ATTR_OPERATION_RESAMPLE_FWD_ALPHA = 1713,
  CUDNN_ATTR_OPERATION_RESAMPLE_FWD_BETA = 1714,
  CUDNN_ATTR_OPERATION_RESAMPLE_FWD_DESC = 1716,
  CUDNN_ATTR_OPERATION_RESAMPLE_BWD_DXDESC = 1720,
  CUDNN_ATTR_OPERATION_RESAMPLE_BWD_DYDESC = 1721,
  CUDNN_ATTR_OPERATION_RESAMPLE_BWD_IDXDESC = 1722,
  CUDNN_ATTR_OPERATION_RESAMPLE_BWD_ALPHA = 1723,
  CUDNN_ATTR_OPERATION_RESAMPLE_BWD_BETA = 1724,
  CUDNN_ATTR_OPERATION_RESAMPLE_BWD_DESC = 1725,
  CUDNN_ATTR_OPERATION_RESAMPLE_BWD_XDESC = 1726,
  CUDNN_ATTR_OPERATION_RESAMPLE_BWD_YDESC = 1727,
  CUDNN_ATTR_OPERATION_CONCAT_AXIS = 1800,
  CUDNN_ATTR_OPERATION_CONCAT_INPUT_DESCS = 1801,
  CUDNN_ATTR_OPERATION_CONCAT_INPLACE_INDEX = 1802,
  CUDNN_ATTR_OPERATION_CONCAT_OUTPUT_DESC = 1803,
  CUDNN_ATTR_OPERATION_SIGNAL_MODE = 1900,
  CUDNN_ATTR_OPERATION_SIGNAL_FLAGDESC = 1901,
  CUDNN_ATTR_OPERATION_SIGNAL_VALUE = 1902,
  CUDNN_ATTR_OPERATION_SIGNAL_XDESC = 1903,
  CUDNN_ATTR_OPERATION_SIGNAL_YDESC = 1904,
  CUDNN_ATTR_OPERATION_NORM_FWD_MODE = 2000,
  CUDNN_ATTR_OPERATION_NORM_FWD_PHASE = 2001,
  CUDNN_ATTR_OPERATION_NORM_FWD_XDESC = 2002,
  CUDNN_ATTR_OPERATION_NORM_FWD_MEAN_DESC = 2003,
  CUDNN_ATTR_OPERATION_NORM_FWD_INV_VARIANCE_DESC = 2004,
  CUDNN_ATTR_OPERATION_NORM_FWD_SCALE_DESC = 2005,
  CUDNN_ATTR_OPERATION_NORM_FWD_BIAS_DESC = 2006,
  CUDNN_ATTR_OPERATION_NORM_FWD_EPSILON_DESC = 2007,
  CUDNN_ATTR_OPERATION_NORM_FWD_EXP_AVG_FACTOR_DESC = 2008,
  CUDNN_ATTR_OPERATION_NORM_FWD_INPUT_RUNNING_MEAN_DESC = 2009,
  CUDNN_ATTR_OPERATION_NORM_FWD_INPUT_RUNNING_VAR_DESC = 2010,
  CUDNN_ATTR_OPERATION_NORM_FWD_OUTPUT_RUNNING_MEAN_DESC = 2011,
  CUDNN_ATTR_OPERATION_NORM_FWD_OUTPUT_RUNNING_VAR_DESC = 2012,
  CUDNN_ATTR_OPERATION_NORM_FWD_YDESC = 2013,
  CUDNN_ATTR_OPERATION_NORM_FWD_PEER_STAT_DESCS = 2014,
  CUDNN_ATTR_OPERATION_NORM_BWD_MODE = 2100,
  CUDNN_ATTR_OPERATION_NORM_BWD_XDESC = 2101,
  CUDNN_ATTR_OPERATION_NORM_BWD_MEAN_DESC = 2102,
  CUDNN_ATTR_OPERATION_NORM_BWD_INV_VARIANCE_DESC = 2103,
  CUDNN_ATTR_OPERATION_NORM_BWD_DYDESC = 2104,
  CUDNN_ATTR_OPERATION_NORM_BWD_SCALE_DESC = 2105,
  CUDNN_ATTR_OPERATION_NORM_BWD_EPSILON_DESC = 2106,
  CUDNN_ATTR_OPERATION_NORM_BWD_DSCALE_DESC = 2107,
  CUDNN_ATTR_OPERATION_NORM_BWD_DBIAS_DESC = 2108,
  CUDNN_ATTR_OPERATION_NORM_BWD_DXDESC = 2109,
  CUDNN_ATTR_OPERATION_NORM_BWD_PEER_STAT_DESCS = 2110,
  CUDNN_ATTR_OPERATION_RESHAPE_XDESC = 2200,
  CUDNN_ATTR_OPERATION_RESHAPE_YDESC = 2201,
  CUDNN_ATTR_RNG_DISTRIBUTION = 2300,
  CUDNN_ATTR_RNG_NORMAL_DIST_MEAN = 2301,
  CUDNN_ATTR_RNG_NORMAL_DIST_STANDARD_DEVIATION = 2302,
  CUDNN_ATTR_RNG_UNIFORM_DIST_MAXIMUM = 2303,
  CUDNN_ATTR_RNG_UNIFORM_DIST_MINIMUM = 2304,
  CUDNN_ATTR_RNG_BERNOULLI_DIST_PROBABILITY = 2305,
  CUDNN_ATTR_OPERATION_RNG_YDESC = 2310,
  CUDNN_ATTR_OPERATION_RNG_SEED = 2311,
  CUDNN_ATTR_OPERATION_RNG_DESC = 2312,
  CUDNN_ATTR_OPERATION_RNG_OFFSET_DESC = 2313,
} cudnnBackendAttributeName_t;

typedef enum  {
  CUDNN_TYPE_HANDLE = 0,
  CUDNN_TYPE_DATA_TYPE,
  CUDNN_TYPE_BOOLEAN,
  CUDNN_TYPE_INT64,
  CUDNN_TYPE_FLOAT,
  CUDNN_TYPE_DOUBLE,
  CUDNN_TYPE_VOID_PTR,
  CUDNN_TYPE_CONVOLUTION_MODE,
  CUDNN_TYPE_HEUR_MODE,
  CUDNN_TYPE_KNOB_TYPE,
  CUDNN_TYPE_NAN_PROPOGATION,
  CUDNN_TYPE_NUMERICAL_NOTE,
  CUDNN_TYPE_LAYOUT_TYPE,
  CUDNN_TYPE_ATTRIB_NAME,
  CUDNN_TYPE_POINTWISE_MODE,
  CUDNN_TYPE_BACKEND_DESCRIPTOR,
  CUDNN_TYPE_GENSTATS_MODE,
  CUDNN_TYPE_BN_FINALIZE_STATS_MODE,
  CUDNN_TYPE_REDUCTION_OPERATOR_TYPE,
  CUDNN_TYPE_BEHAVIOR_NOTE,
  CUDNN_TYPE_TENSOR_REORDERING_MODE,
  CUDNN_TYPE_RESAMPLE_MODE,
  CUDNN_TYPE_PADDING_MODE,
  CUDNN_TYPE_INT32,
  CUDNN_TYPE_CHAR,
  CUDNN_TYPE_SIGNAL_MODE,
  CUDNN_TYPE_FRACTION,
  CUDNN_TYPE_NORM_MODE,
  CUDNN_TYPE_NORM_FWD_PHASE,
  CUDNN_TYPE_RNG_DISTRIBUTION,
} cudnnBackendAttributeType_t;

typedef enum  {
  CUDNN_BACKEND_POINTWISE_DESCRIPTOR = 0,
  CUDNN_BACKEND_CONVOLUTION_DESCRIPTOR,
  CUDNN_BACKEND_ENGINE_DESCRIPTOR,
  CUDNN_BACKEND_ENGINECFG_DESCRIPTOR,
  CUDNN_BACKEND_ENGINEHEUR_DESCRIPTOR,
  CUDNN_BACKEND_EXECUTION_PLAN_DESCRIPTOR,
  CUDNN_BACKEND_INTERMEDIATE_INFO_DESCRIPTOR,
  CUDNN_BACKEND_KNOB_CHOICE_DESCRIPTOR,
  CUDNN_BACKEND_KNOB_INFO_DESCRIPTOR,
  CUDNN_BACKEND_LAYOUT_INFO_DESCRIPTOR,
  CUDNN_BACKEND_OPERATION_CONVOLUTION_FORWARD_DESCRIPTOR,
  CUDNN_BACKEND_OPERATION_CONVOLUTION_BACKWARD_FILTER_DESCRIPTOR,
  CUDNN_BACKEND_OPERATION_CONVOLUTION_BACKWARD_DATA_DESCRIPTOR,
  CUDNN_BACKEND_OPERATION_POINTWISE_DESCRIPTOR,
  CUDNN_BACKEND_OPERATION_GEN_STATS_DESCRIPTOR,
  CUDNN_BACKEND_OPERATIONGRAPH_DESCRIPTOR,
  CUDNN_BACKEND_VARIANT_PACK_DESCRIPTOR,
  CUDNN_BACKEND_TENSOR_DESCRIPTOR,
  CUDNN_BACKEND_MATMUL_DESCRIPTOR,
  CUDNN_BACKEND_OPERATION_MATMUL_DESCRIPTOR,
  CUDNN_BACKEND_OPERATION_BN_FINALIZE_STATISTICS_DESCRIPTOR,
  CUDNN_BACKEND_REDUCTION_DESCRIPTOR,
  CUDNN_BACKEND_OPERATION_REDUCTION_DESCRIPTOR,
  CUDNN_BACKEND_OPERATION_BN_BWD_WEIGHTS_DESCRIPTOR,
  CUDNN_BACKEND_RESAMPLE_DESCRIPTOR,
  CUDNN_BACKEND_OPERATION_RESAMPLE_FWD_DESCRIPTOR,
  CUDNN_BACKEND_OPERATION_RESAMPLE_BWD_DESCRIPTOR,
  CUDNN_BACKEND_OPERATION_CONCAT_DESCRIPTOR,
  CUDNN_BACKEND_OPERATION_SIGNAL_DESCRIPTOR,
  CUDNN_BACKEND_OPERATION_NORM_FORWARD_DESCRIPTOR,
  CUDNN_BACKEND_OPERATION_NORM_BACKWARD_DESCRIPTOR,
  CUDNN_BACKEND_OPERATION_RESHAPE_DESCRIPTOR,
  CUDNN_BACKEND_RNG_DESCRIPTOR,
  CUDNN_BACKEND_OPERATION_RNG_DESCRIPTOR,
} cudnnBackendDescriptorType_t;

typedef enum  {
  CUDNN_NUMERICAL_NOTE_TENSOR_CORE = 0,
  CUDNN_NUMERICAL_NOTE_DOWN_CONVERT_INPUTS,
  CUDNN_NUMERICAL_NOTE_REDUCED_PRECISION_REDUCTION,
  CUDNN_NUMERICAL_NOTE_FFT,
  CUDNN_NUMERICAL_NOTE_NONDETERMINISTIC,
  CUDNN_NUMERICAL_NOTE_WINOGRAD,
  CUDNN_NUMERICAL_NOTE_WINOGRAD_TILE_4x4,
  CUDNN_NUMERICAL_NOTE_WINOGRAD_TILE_6x6,
  CUDNN_NUMERICAL_NOTE_WINOGRAD_TILE_13x13,
  CUDNN_NUMERICAL_NOTE_TYPE_COUNT,
} cudnnBackendNumericalNote_t;

typedef enum  {
  CUDNN_BEHAVIOR_NOTE_RUNTIME_COMPILATION = 0,
  CUDNN_BEHAVIOR_NOTE_REQUIRES_FILTER_INT8x32_REORDER = 1,
  CUDNN_BEHAVIOR_NOTE_REQUIRES_BIAS_INT8x32_REORDER = 2,
  CUDNN_BEHAVIOR_NOTE_TYPE_COUNT,
} cudnnBackendBehaviorNote_t;

typedef enum  {
  CUDNN_KNOB_TYPE_SPLIT_K = 0,
  CUDNN_KNOB_TYPE_SWIZZLE = 1,
  CUDNN_KNOB_TYPE_TILE_SIZE = 2,
  CUDNN_KNOB_TYPE_USE_TEX = 3,
  CUDNN_KNOB_TYPE_EDGE = 4,
  CUDNN_KNOB_TYPE_KBLOCK = 5,
  CUDNN_KNOB_TYPE_LDGA = 6,
  CUDNN_KNOB_TYPE_LDGB = 7,
  CUDNN_KNOB_TYPE_CHUNK_K = 8,
  CUDNN_KNOB_TYPE_SPLIT_H = 9,
  CUDNN_KNOB_TYPE_WINO_TILE = 10,
  CUDNN_KNOB_TYPE_MULTIPLY = 11,
  CUDNN_KNOB_TYPE_SPLIT_K_BUF = 12,
  CUDNN_KNOB_TYPE_TILEK = 13,
  CUDNN_KNOB_TYPE_STAGES = 14,
  CUDNN_KNOB_TYPE_REDUCTION_MODE = 15,
  CUDNN_KNOB_TYPE_CTA_SPLIT_K_MODE = 16,
  CUDNN_KNOB_TYPE_SPLIT_K_SLC = 17,
  CUDNN_KNOB_TYPE_IDX_MODE = 18,
  CUDNN_KNOB_TYPE_SLICED = 19,
  CUDNN_KNOB_TYPE_SPLIT_RS = 20,
  CUDNN_KNOB_TYPE_SINGLEBUFFER = 21,
  CUDNN_KNOB_TYPE_LDGC = 22,
  CUDNN_KNOB_TYPE_SPECFILT = 23,
  CUDNN_KNOB_TYPE_KERNEL_CFG = 24,
  CUDNN_KNOB_TYPE_WORKSPACE = 25,
  CUDNN_KNOB_TYPE_TILE_CGA = 26,
  CUDNN_KNOB_TYPE_TILE_CGA_M = 27,
  CUDNN_KNOB_TYPE_TILE_CGA_N = 28,
  CUDNN_KNOB_TYPE_BLOCK_SIZE = 29,
  CUDNN_KNOB_TYPE_OCCUPANCY = 30,
  CUDNN_KNOB_TYPE_ARRAY_SIZE_PER_THREAD = 31,
  CUDNN_KNOB_TYPE_NUM_C_PER_BLOCK = 32,
  CUDNN_KNOB_TYPE_COUNTS,
} cudnnBackendKnobType_t;

typedef enum  {
  CUDNN_LAYOUT_TYPE_PREFERRED_NCHW = 0,
  CUDNN_LAYOUT_TYPE_PREFERRED_NHWC = 1,
  CUDNN_LAYOUT_TYPE_PREFERRED_PAD4CK = 2,
  CUDNN_LAYOUT_TYPE_PREFERRED_PAD8CK = 3,
  CUDNN_LAYOUT_TYPE_COUNT = 4,
} cudnnBackendLayoutType_t;

typedef enum  {
  CUDNN_HEUR_MODE_INSTANT = 0,
  CUDNN_HEUR_MODE_B = 1,
  CUDNN_HEUR_MODE_FALLBACK = 2,
  CUDNN_HEUR_MODE_A = 3,
  CUDNN_HEUR_MODES_COUNT = 4,
} cudnnBackendHeurMode_t;

typedef enum  {
  CUDNN_TENSOR_REORDERING_NONE = 0,
  CUDNN_TENSOR_REORDERING_INT8x32 = 1,
  CUDNN_TENSOR_REORDERING_F16x16 = 2,
} cudnnBackendTensorReordering_t;

typedef enum  {
  CUDNN_ZERO_PAD = 0,
  CUDNN_NEG_INF_PAD = 1,
  CUDNN_EDGE_VAL_PAD = 2,
} cudnnPaddingMode_t;

typedef enum  {
  CUDNN_LAYER_NORM = 0,
  CUDNN_INSTANCE_NORM = 1,
  CUDNN_BATCH_NORM = 2,
  CUDNN_GROUP_NORM = 3,
} cudnnBackendNormMode_t;

typedef enum  {
  CUDNN_NORM_FWD_INFERENCE = 0,
  CUDNN_NORM_FWD_TRAINING = 1,
} cudnnBackendNormFwdPhase_t;
typedef unsigned int GLenum;
typedef unsigned int GLuint;
typedef int GLint;

typedef enum CUGLDeviceList_enum {
  CU_GL_DEVICE_LIST_ALL = 0x01,
  CU_GL_DEVICE_LIST_CURRENT_FRAME = 0x02,
  CU_GL_DEVICE_LIST_NEXT_FRAME = 0x03,
} CUGLDeviceList;

typedef enum CUGLmap_flags_enum {
  CU_GL_MAP_RESOURCE_FLAGS_NONE = 0x00,
  CU_GL_MAP_RESOURCE_FLAGS_READ_ONLY = 0x01,
  CU_GL_MAP_RESOURCE_FLAGS_WRITE_DISCARD = 0x02,
} CUGLmap_flags;


/* Function types. */
typedef CUresult CUDAAPI tcuGetErrorString(CUresult error, const char** pStr);
typedef CUresult CUDAAPI tcuGetErrorName(CUresult error, const char** pStr);
typedef CUresult CUDAAPI tcuInit(unsigned int Flags);
typedef CUresult CUDAAPI tcuDriverGetVersion(int* driverVersion);
typedef CUresult CUDAAPI tcuDeviceGet(CUdevice* device, int ordinal);
typedef CUresult CUDAAPI tcuDeviceGetCount(int* count);
typedef CUresult CUDAAPI tcuDeviceGetName(char* name, int len, CUdevice dev);
typedef CUresult CUDAAPI tcuDeviceGetUuid(CUuuid* uuid, CUdevice dev);
typedef CUresult CUDAAPI tcuDeviceGetUuid_v2(CUuuid* uuid, CUdevice dev);
typedef CUresult CUDAAPI tcuDeviceGetLuid(char* luid, unsigned int* deviceNodeMask, CUdevice dev);
typedef CUresult CUDAAPI tcuDeviceTotalMem_v2(size_t* bytes, CUdevice dev);
typedef CUresult CUDAAPI tcuDeviceGetTexture1DLinearMaxWidth(size_t* maxWidthInElements, CUarray_format format, unsigned numChannels, CUdevice dev);
typedef CUresult CUDAAPI tcuDeviceGetAttribute(int* pi, CUdevice_attribute attrib, CUdevice dev);
typedef CUresult CUDAAPI tcuDeviceGetNvSciSyncAttributes(void* nvSciSyncAttrList, CUdevice dev, int flags);
typedef CUresult CUDAAPI tcuDeviceSetMemPool(CUdevice dev, CUmemoryPool pool);
typedef CUresult CUDAAPI tcuDeviceGetMemPool(CUmemoryPool* pool, CUdevice dev);
typedef CUresult CUDAAPI tcuDeviceGetDefaultMemPool(CUmemoryPool* pool_out, CUdevice dev);
typedef CUresult CUDAAPI tcuDeviceGetExecAffinitySupport(int* pi, CUexecAffinityType type, CUdevice dev);
typedef CUresult CUDAAPI tcuFlushGPUDirectRDMAWrites(CUflushGPUDirectRDMAWritesTarget target, CUflushGPUDirectRDMAWritesScope scope);
typedef CUresult CUDAAPI tcuDeviceGetProperties(CUdevprop* prop, CUdevice dev);
typedef CUresult CUDAAPI tcuDeviceComputeCapability(int* major, int* minor, CUdevice dev);
typedef CUresult CUDAAPI tcuDevicePrimaryCtxRetain(CUcontext* pctx, CUdevice dev);
typedef CUresult CUDAAPI tcuDevicePrimaryCtxRelease_v2(CUdevice dev);
typedef CUresult CUDAAPI tcuDevicePrimaryCtxSetFlags_v2(CUdevice dev, unsigned int flags);
typedef CUresult CUDAAPI tcuDevicePrimaryCtxGetState(CUdevice dev, unsigned int* flags, int* active);
typedef CUresult CUDAAPI tcuDevicePrimaryCtxReset_v2(CUdevice dev);
typedef CUresult CUDAAPI tcuCtxCreate_v2(CUcontext* pctx, unsigned int flags, CUdevice dev);
typedef CUresult CUDAAPI tcuCtxCreate_v3(CUcontext* pctx, CUexecAffinityParam* paramsArray, int numParams, unsigned int flags, CUdevice dev);
typedef CUresult CUDAAPI tcuCtxDestroy_v2(CUcontext ctx);
typedef CUresult CUDAAPI tcuCtxPushCurrent_v2(CUcontext ctx);
typedef CUresult CUDAAPI tcuCtxPopCurrent_v2(CUcontext* pctx);
typedef CUresult CUDAAPI tcuCtxSetCurrent(CUcontext ctx);
typedef CUresult CUDAAPI tcuCtxGetCurrent(CUcontext* pctx);
typedef CUresult CUDAAPI tcuCtxGetDevice(CUdevice* device);
typedef CUresult CUDAAPI tcuCtxGetFlags(unsigned int* flags);
typedef CUresult CUDAAPI tcuCtxSetFlags(unsigned int flags);
typedef CUresult CUDAAPI tcuCtxGetId(CUcontext ctx, unsigned long long* ctxId);
typedef CUresult CUDAAPI tcuCtxSynchronize(void);
typedef CUresult CUDAAPI tcuCtxSetLimit(CUlimit limit, size_t value);
typedef CUresult CUDAAPI tcuCtxGetLimit(size_t* pvalue, CUlimit limit);
typedef CUresult CUDAAPI tcuCtxGetCacheConfig(CUfunc_cache* pconfig);
typedef CUresult CUDAAPI tcuCtxSetCacheConfig(CUfunc_cache config);
typedef CUresult CUDAAPI tcuCtxGetSharedMemConfig(CUsharedconfig* pConfig);
typedef CUresult CUDAAPI tcuCtxSetSharedMemConfig(CUsharedconfig config);
typedef CUresult CUDAAPI tcuCtxGetApiVersion(CUcontext ctx, unsigned int* version);
typedef CUresult CUDAAPI tcuCtxGetStreamPriorityRange(int* leastPriority, int* greatestPriority);
typedef CUresult CUDAAPI tcuCtxResetPersistingL2Cache(void);
typedef CUresult CUDAAPI tcuCtxGetExecAffinity(CUexecAffinityParam* pExecAffinity, CUexecAffinityType type);
typedef CUresult CUDAAPI tcuCtxAttach(CUcontext* pctx, unsigned int flags);
typedef CUresult CUDAAPI tcuCtxDetach(CUcontext ctx);
typedef CUresult CUDAAPI tcuModuleLoad(CUmodule* module, const char* fname);
typedef CUresult CUDAAPI tcuModuleLoadData(CUmodule* module, const void* image);
typedef CUresult CUDAAPI tcuModuleLoadDataEx(CUmodule* module, const void* image, unsigned int numOptions, CUjit_option* options, void** optionValues);
typedef CUresult CUDAAPI tcuModuleLoadFatBinary(CUmodule* module, const void* fatCubin);
typedef CUresult CUDAAPI tcuModuleUnload(CUmodule hmod);
typedef CUresult CUDAAPI tcuModuleGetLoadingMode(CUmoduleLoadingMode* mode);
typedef CUresult CUDAAPI tcuModuleGetFunction(CUfunction* hfunc, CUmodule hmod, const char* name);
typedef CUresult CUDAAPI tcuModuleGetGlobal_v2(CUdeviceptr* dptr, size_t* bytes, CUmodule hmod, const char* name);
typedef CUresult CUDAAPI tcuLinkCreate_v2(unsigned int numOptions, CUjit_option* options, void** optionValues, CUlinkState* stateOut);
typedef CUresult CUDAAPI tcuLinkAddData_v2(CUlinkState state, CUjitInputType type, void* data, size_t size, const char* name, unsigned int numOptions, CUjit_option* options, void** optionValues);
typedef CUresult CUDAAPI tcuLinkAddFile_v2(CUlinkState state, CUjitInputType type, const char* path, unsigned int numOptions, CUjit_option* options, void** optionValues);
typedef CUresult CUDAAPI tcuLinkComplete(CUlinkState state, void** cubinOut, size_t* sizeOut);
typedef CUresult CUDAAPI tcuLinkDestroy(CUlinkState state);
typedef CUresult CUDAAPI tcuModuleGetTexRef(CUtexref* pTexRef, CUmodule hmod, const char* name);
typedef CUresult CUDAAPI tcuModuleGetSurfRef(CUsurfref* pSurfRef, CUmodule hmod, const char* name);
typedef CUresult CUDAAPI tcuLibraryLoadData(CUlibrary* library, const void* code, CUjit_option* jitOptions, void** jitOptionsValues, unsigned int numJitOptions, CUlibraryOption* libraryOptions, void** libraryOptionValues, unsigned int numLibraryOptions);
typedef CUresult CUDAAPI tcuLibraryLoadFromFile(CUlibrary* library, const char* fileName, CUjit_option* jitOptions, void** jitOptionsValues, unsigned int numJitOptions, CUlibraryOption* libraryOptions, void** libraryOptionValues, unsigned int numLibraryOptions);
typedef CUresult CUDAAPI tcuLibraryUnload(CUlibrary library);
typedef CUresult CUDAAPI tcuLibraryGetKernel(CUkernel* pKernel, CUlibrary library, const char* name);
typedef CUresult CUDAAPI tcuLibraryGetModule(CUmodule* pMod, CUlibrary library);
typedef CUresult CUDAAPI tcuKernelGetFunction(CUfunction* pFunc, CUkernel kernel);
typedef CUresult CUDAAPI tcuLibraryGetGlobal(CUdeviceptr* dptr, size_t* bytes, CUlibrary library, const char* name);
typedef CUresult CUDAAPI tcuLibraryGetManaged(CUdeviceptr* dptr, size_t* bytes, CUlibrary library, const char* name);
typedef CUresult CUDAAPI tcuLibraryGetUnifiedFunction(void** fptr, CUlibrary library, const char* symbol);
typedef CUresult CUDAAPI tcuKernelGetAttribute(int* pi, CUfunction_attribute attrib, CUkernel kernel, CUdevice dev);
typedef CUresult CUDAAPI tcuKernelSetAttribute(CUfunction_attribute attrib, int val, CUkernel kernel, CUdevice dev);
typedef CUresult CUDAAPI tcuKernelSetCacheConfig(CUkernel kernel, CUfunc_cache config, CUdevice dev);
typedef CUresult CUDAAPI tcuMemGetInfo_v2(size_t* free, size_t* total);
typedef CUresult CUDAAPI tcuMemAlloc_v2(CUdeviceptr* dptr, size_t bytesize);
typedef CUresult CUDAAPI tcuMemAllocPitch_v2(CUdeviceptr* dptr, size_t* pPitch, size_t WidthInBytes, size_t Height, unsigned int ElementSizeBytes);
typedef CUresult CUDAAPI tcuMemFree_v2(CUdeviceptr dptr);
typedef CUresult CUDAAPI tcuMemGetAddressRange_v2(CUdeviceptr* pbase, size_t* psize, CUdeviceptr dptr);
typedef CUresult CUDAAPI tcuMemAllocHost_v2(void** pp, size_t bytesize);
typedef CUresult CUDAAPI tcuMemFreeHost(void* p);
typedef CUresult CUDAAPI tcuMemHostAlloc(void** pp, size_t bytesize, unsigned int Flags);
typedef CUresult CUDAAPI tcuMemHostGetDevicePointer_v2(CUdeviceptr* pdptr, void* p, unsigned int Flags);
typedef CUresult CUDAAPI tcuMemHostGetFlags(unsigned int* pFlags, void* p);
typedef CUresult CUDAAPI tcuMemAllocManaged(CUdeviceptr* dptr, size_t bytesize, unsigned int flags);
typedef CUresult CUDAAPI tcuDeviceGetByPCIBusId(CUdevice* dev, const char* pciBusId);
typedef CUresult CUDAAPI tcuDeviceGetPCIBusId(char* pciBusId, int len, CUdevice dev);
typedef CUresult CUDAAPI tcuIpcGetEventHandle(CUipcEventHandle* pHandle, CUevent event);
typedef CUresult CUDAAPI tcuIpcOpenEventHandle(CUevent* phEvent, CUipcEventHandle handle);
typedef CUresult CUDAAPI tcuIpcGetMemHandle(CUipcMemHandle* pHandle, CUdeviceptr dptr);
typedef CUresult CUDAAPI tcuIpcOpenMemHandle_v2(CUdeviceptr* pdptr, CUipcMemHandle handle, unsigned int Flags);
typedef CUresult CUDAAPI tcuIpcCloseMemHandle(CUdeviceptr dptr);
typedef CUresult CUDAAPI tcuMemHostRegister_v2(void* p, size_t bytesize, unsigned int Flags);
typedef CUresult CUDAAPI tcuMemHostUnregister(void* p);
typedef CUresult CUDAAPI tcuMemcpy(CUdeviceptr dst, CUdeviceptr src, size_t ByteCount);
typedef CUresult CUDAAPI tcuMemcpyPeer(CUdeviceptr dstDevice, CUcontext dstContext, CUdeviceptr srcDevice, CUcontext srcContext, size_t ByteCount);
typedef CUresult CUDAAPI tcuMemcpyHtoD_v2(CUdeviceptr dstDevice, const void* srcHost, size_t ByteCount);
typedef CUresult CUDAAPI tcuMemcpyDtoH_v2(void* dstHost, CUdeviceptr srcDevice, size_t ByteCount);
typedef CUresult CUDAAPI tcuMemcpyDtoD_v2(CUdeviceptr dstDevice, CUdeviceptr srcDevice, size_t ByteCount);
typedef CUresult CUDAAPI tcuMemcpyDtoA_v2(CUarray dstArray, size_t dstOffset, CUdeviceptr srcDevice, size_t ByteCount);
typedef CUresult CUDAAPI tcuMemcpyAtoD_v2(CUdeviceptr dstDevice, CUarray srcArray, size_t srcOffset, size_t ByteCount);
typedef CUresult CUDAAPI tcuMemcpyHtoA_v2(CUarray dstArray, size_t dstOffset, const void* srcHost, size_t ByteCount);
typedef CUresult CUDAAPI tcuMemcpyAtoH_v2(void* dstHost, CUarray srcArray, size_t srcOffset, size_t ByteCount);
typedef CUresult CUDAAPI tcuMemcpyAtoA_v2(CUarray dstArray, size_t dstOffset, CUarray srcArray, size_t srcOffset, size_t ByteCount);
typedef CUresult CUDAAPI tcuMemcpy2D_v2(const CUDA_MEMCPY2D* pCopy);
typedef CUresult CUDAAPI tcuMemcpy2DUnaligned_v2(const CUDA_MEMCPY2D* pCopy);
typedef CUresult CUDAAPI tcuMemcpy3D_v2(const CUDA_MEMCPY3D* pCopy);
typedef CUresult CUDAAPI tcuMemcpy3DPeer(const CUDA_MEMCPY3D_PEER* pCopy);
typedef CUresult CUDAAPI tcuMemcpyAsync(CUdeviceptr dst, CUdeviceptr src, size_t ByteCount, CUstream hStream);
typedef CUresult CUDAAPI tcuMemcpyPeerAsync(CUdeviceptr dstDevice, CUcontext dstContext, CUdeviceptr srcDevice, CUcontext srcContext, size_t ByteCount, CUstream hStream);
typedef CUresult CUDAAPI tcuMemcpyHtoDAsync_v2(CUdeviceptr dstDevice, const void* srcHost, size_t ByteCount, CUstream hStream);
typedef CUresult CUDAAPI tcuMemcpyDtoHAsync_v2(void* dstHost, CUdeviceptr srcDevice, size_t ByteCount, CUstream hStream);
typedef CUresult CUDAAPI tcuMemcpyDtoDAsync_v2(CUdeviceptr dstDevice, CUdeviceptr srcDevice, size_t ByteCount, CUstream hStream);
typedef CUresult CUDAAPI tcuMemcpyHtoAAsync_v2(CUarray dstArray, size_t dstOffset, const void* srcHost, size_t ByteCount, CUstream hStream);
typedef CUresult CUDAAPI tcuMemcpyAtoHAsync_v2(void* dstHost, CUarray srcArray, size_t srcOffset, size_t ByteCount, CUstream hStream);
typedef CUresult CUDAAPI tcuMemcpy2DAsync_v2(const CUDA_MEMCPY2D* pCopy, CUstream hStream);
typedef CUresult CUDAAPI tcuMemcpy3DAsync_v2(const CUDA_MEMCPY3D* pCopy, CUstream hStream);
typedef CUresult CUDAAPI tcuMemcpy3DPeerAsync(const CUDA_MEMCPY3D_PEER* pCopy, CUstream hStream);
typedef CUresult CUDAAPI tcuMemsetD8_v2(CUdeviceptr dstDevice, unsigned char uc, size_t N);
typedef CUresult CUDAAPI tcuMemsetD16_v2(CUdeviceptr dstDevice, unsigned short us, size_t N);
typedef CUresult CUDAAPI tcuMemsetD32_v2(CUdeviceptr dstDevice, unsigned int ui, size_t N);
typedef CUresult CUDAAPI tcuMemsetD2D8_v2(CUdeviceptr dstDevice, size_t dstPitch, unsigned char uc, size_t Width, size_t Height);
typedef CUresult CUDAAPI tcuMemsetD2D16_v2(CUdeviceptr dstDevice, size_t dstPitch, unsigned short us, size_t Width, size_t Height);
typedef CUresult CUDAAPI tcuMemsetD2D32_v2(CUdeviceptr dstDevice, size_t dstPitch, unsigned int ui, size_t Width, size_t Height);
typedef CUresult CUDAAPI tcuMemsetD8Async(CUdeviceptr dstDevice, unsigned char uc, size_t N, CUstream hStream);
typedef CUresult CUDAAPI tcuMemsetD16Async(CUdeviceptr dstDevice, unsigned short us, size_t N, CUstream hStream);
typedef CUresult CUDAAPI tcuMemsetD32Async(CUdeviceptr dstDevice, unsigned int ui, size_t N, CUstream hStream);
typedef CUresult CUDAAPI tcuMemsetD2D8Async(CUdeviceptr dstDevice, size_t dstPitch, unsigned char uc, size_t Width, size_t Height, CUstream hStream);
typedef CUresult CUDAAPI tcuMemsetD2D16Async(CUdeviceptr dstDevice, size_t dstPitch, unsigned short us, size_t Width, size_t Height, CUstream hStream);
typedef CUresult CUDAAPI tcuMemsetD2D32Async(CUdeviceptr dstDevice, size_t dstPitch, unsigned int ui, size_t Width, size_t Height, CUstream hStream);
typedef CUresult CUDAAPI tcuArrayCreate_v2(CUarray* pHandle, const CUDA_ARRAY_DESCRIPTOR* pAllocateArray);
typedef CUresult CUDAAPI tcuArrayGetDescriptor_v2(CUDA_ARRAY_DESCRIPTOR* pArrayDescriptor, CUarray hArray);
typedef CUresult CUDAAPI tcuArrayGetSparseProperties(CUDA_ARRAY_SPARSE_PROPERTIES* sparseProperties, CUarray array);
typedef CUresult CUDAAPI tcuMipmappedArrayGetSparseProperties(CUDA_ARRAY_SPARSE_PROPERTIES* sparseProperties, CUmipmappedArray mipmap);
typedef CUresult CUDAAPI tcuArrayGetMemoryRequirements(CUDA_ARRAY_MEMORY_REQUIREMENTS* memoryRequirements, CUarray array, CUdevice device);
typedef CUresult CUDAAPI tcuMipmappedArrayGetMemoryRequirements(CUDA_ARRAY_MEMORY_REQUIREMENTS* memoryRequirements, CUmipmappedArray mipmap, CUdevice device);
typedef CUresult CUDAAPI tcuArrayGetPlane(CUarray* pPlaneArray, CUarray hArray, unsigned int planeIdx);
typedef CUresult CUDAAPI tcuArrayDestroy(CUarray hArray);
typedef CUresult CUDAAPI tcuArray3DCreate_v2(CUarray* pHandle, const CUDA_ARRAY3D_DESCRIPTOR* pAllocateArray);
typedef CUresult CUDAAPI tcuArray3DGetDescriptor_v2(CUDA_ARRAY3D_DESCRIPTOR* pArrayDescriptor, CUarray hArray);
typedef CUresult CUDAAPI tcuMipmappedArrayCreate(CUmipmappedArray* pHandle, const CUDA_ARRAY3D_DESCRIPTOR* pMipmappedArrayDesc, unsigned int numMipmapLevels);
typedef CUresult CUDAAPI tcuMipmappedArrayGetLevel(CUarray* pLevelArray, CUmipmappedArray hMipmappedArray, unsigned int level);
typedef CUresult CUDAAPI tcuMipmappedArrayDestroy(CUmipmappedArray hMipmappedArray);
typedef CUresult CUDAAPI tcuMemGetHandleForAddressRange(void* handle, CUdeviceptr dptr, size_t size, CUmemRangeHandleType handleType, unsigned long long flags);
typedef CUresult CUDAAPI tcuMemAddressReserve(CUdeviceptr* ptr, size_t size, size_t alignment, CUdeviceptr addr, unsigned long long flags);
typedef CUresult CUDAAPI tcuMemAddressFree(CUdeviceptr ptr, size_t size);
typedef CUresult CUDAAPI tcuMemCreate(CUmemGenericAllocationHandle* handle, size_t size, const CUmemAllocationProp* prop, unsigned long long flags);
typedef CUresult CUDAAPI tcuMemRelease(CUmemGenericAllocationHandle handle);
typedef CUresult CUDAAPI tcuMemMap(CUdeviceptr ptr, size_t size, size_t offset, CUmemGenericAllocationHandle handle, unsigned long long flags);
typedef CUresult CUDAAPI tcuMemMapArrayAsync(CUarrayMapInfo* mapInfoList, unsigned int count, CUstream hStream);
typedef CUresult CUDAAPI tcuMemUnmap(CUdeviceptr ptr, size_t size);
typedef CUresult CUDAAPI tcuMemSetAccess(CUdeviceptr ptr, size_t size, const CUmemAccessDesc* desc, size_t count);
typedef CUresult CUDAAPI tcuMemGetAccess(unsigned long long* flags, const CUmemLocation* location, CUdeviceptr ptr);
typedef CUresult CUDAAPI tcuMemExportToShareableHandle(void* shareableHandle, CUmemGenericAllocationHandle handle, CUmemAllocationHandleType handleType, unsigned long long flags);
typedef CUresult CUDAAPI tcuMemImportFromShareableHandle(CUmemGenericAllocationHandle* handle, void* osHandle, CUmemAllocationHandleType shHandleType);
typedef CUresult CUDAAPI tcuMemGetAllocationGranularity(size_t* granularity, const CUmemAllocationProp* prop, CUmemAllocationGranularity_flags option);
typedef CUresult CUDAAPI tcuMemGetAllocationPropertiesFromHandle(CUmemAllocationProp* prop, CUmemGenericAllocationHandle handle);
typedef CUresult CUDAAPI tcuMemRetainAllocationHandle(CUmemGenericAllocationHandle* handle, void* addr);
typedef CUresult CUDAAPI tcuMemFreeAsync(CUdeviceptr dptr, CUstream hStream);
typedef CUresult CUDAAPI tcuMemAllocAsync(CUdeviceptr* dptr, size_t bytesize, CUstream hStream);
typedef CUresult CUDAAPI tcuMemPoolTrimTo(CUmemoryPool pool, size_t minBytesToKeep);
typedef CUresult CUDAAPI tcuMemPoolSetAttribute(CUmemoryPool pool, CUmemPool_attribute attr, void* value);
typedef CUresult CUDAAPI tcuMemPoolGetAttribute(CUmemoryPool pool, CUmemPool_attribute attr, void* value);
typedef CUresult CUDAAPI tcuMemPoolSetAccess(CUmemoryPool pool, const CUmemAccessDesc* map, size_t count);
typedef CUresult CUDAAPI tcuMemPoolGetAccess(CUmemAccess_flags* flags, CUmemoryPool memPool, CUmemLocation* location);
typedef CUresult CUDAAPI tcuMemPoolCreate(CUmemoryPool* pool, const CUmemPoolProps* poolProps);
typedef CUresult CUDAAPI tcuMemPoolDestroy(CUmemoryPool pool);
typedef CUresult CUDAAPI tcuMemAllocFromPoolAsync(CUdeviceptr* dptr, size_t bytesize, CUmemoryPool pool, CUstream hStream);
typedef CUresult CUDAAPI tcuMemPoolExportToShareableHandle(void* handle_out, CUmemoryPool pool, CUmemAllocationHandleType handleType, unsigned long long flags);
typedef CUresult CUDAAPI tcuMemPoolImportFromShareableHandle(CUmemoryPool* pool_out, void* handle, CUmemAllocationHandleType handleType, unsigned long long flags);
typedef CUresult CUDAAPI tcuMemPoolExportPointer(CUmemPoolPtrExportData* shareData_out, CUdeviceptr ptr);
typedef CUresult CUDAAPI tcuMemPoolImportPointer(CUdeviceptr* ptr_out, CUmemoryPool pool, CUmemPoolPtrExportData* shareData);
typedef CUresult CUDAAPI tcuMulticastCreate(CUmemGenericAllocationHandle* mcHandle, const CUmulticastObjectProp* prop);
typedef CUresult CUDAAPI tcuMulticastAddDevice(CUmemGenericAllocationHandle mcHandle, CUdevice dev);
typedef CUresult CUDAAPI tcuMulticastBindMem(CUmemGenericAllocationHandle mcHandle, size_t mcOffset, CUmemGenericAllocationHandle memHandle, size_t memOffset, size_t size, unsigned long long flags);
typedef CUresult CUDAAPI tcuMulticastBindAddr(CUmemGenericAllocationHandle mcHandle, size_t mcOffset, CUdeviceptr memptr, size_t size, unsigned long long flags);
typedef CUresult CUDAAPI tcuMulticastUnbind(CUmemGenericAllocationHandle mcHandle, CUdevice dev, size_t mcOffset, size_t size);
typedef CUresult CUDAAPI tcuMulticastGetGranularity(size_t* granularity, const CUmulticastObjectProp* prop, CUmulticastGranularity_flags option);
typedef CUresult CUDAAPI tcuPointerGetAttribute(void* data, CUpointer_attribute attribute, CUdeviceptr ptr);
typedef CUresult CUDAAPI tcuMemPrefetchAsync(CUdeviceptr devPtr, size_t count, CUdevice dstDevice, CUstream hStream);
typedef CUresult CUDAAPI tcuMemAdvise(CUdeviceptr devPtr, size_t count, CUmem_advise advice, CUdevice device);
typedef CUresult CUDAAPI tcuMemRangeGetAttribute(void* data, size_t dataSize, CUmem_range_attribute attribute, CUdeviceptr devPtr, size_t count);
typedef CUresult CUDAAPI tcuMemRangeGetAttributes(void** data, size_t* dataSizes, CUmem_range_attribute* attributes, size_t numAttributes, CUdeviceptr devPtr, size_t count);
typedef CUresult CUDAAPI tcuPointerSetAttribute(const void* value, CUpointer_attribute attribute, CUdeviceptr ptr);
typedef CUresult CUDAAPI tcuPointerGetAttributes(unsigned int numAttributes, CUpointer_attribute* attributes, void** data, CUdeviceptr ptr);
typedef CUresult CUDAAPI tcuStreamCreate(CUstream* phStream, unsigned int Flags);
typedef CUresult CUDAAPI tcuStreamCreateWithPriority(CUstream* phStream, unsigned int flags, int priority);
typedef CUresult CUDAAPI tcuStreamGetPriority(CUstream hStream, int* priority);
typedef CUresult CUDAAPI tcuStreamGetFlags(CUstream hStream, unsigned int* flags);
typedef CUresult CUDAAPI tcuStreamGetId(CUstream hStream, unsigned long long* streamId);
typedef CUresult CUDAAPI tcuStreamGetCtx(CUstream hStream, CUcontext* pctx);
typedef CUresult CUDAAPI tcuStreamWaitEvent(CUstream hStream, CUevent hEvent, unsigned int Flags);
typedef CUresult CUDAAPI tcuStreamAddCallback(CUstream hStream, CUstreamCallback callback, void* userData, unsigned int flags);
typedef CUresult CUDAAPI tcuStreamBeginCapture_v2(CUstream hStream, CUstreamCaptureMode mode);
typedef CUresult CUDAAPI tcuThreadExchangeStreamCaptureMode(CUstreamCaptureMode* mode);
typedef CUresult CUDAAPI tcuStreamEndCapture(CUstream hStream, CUgraph* phGraph);
typedef CUresult CUDAAPI tcuStreamIsCapturing(CUstream hStream, CUstreamCaptureStatus* captureStatus);
typedef CUresult CUDAAPI tcuStreamGetCaptureInfo_v2(CUstream hStream, CUstreamCaptureStatus* captureStatus_out, cuuint64_t* id_out, CUgraph* graph_out, const CUgraphNode** dependencies_out, size_t* numDependencies_out);
typedef CUresult CUDAAPI tcuStreamUpdateCaptureDependencies(CUstream hStream, CUgraphNode* dependencies, size_t numDependencies, unsigned int flags);
typedef CUresult CUDAAPI tcuStreamAttachMemAsync(CUstream hStream, CUdeviceptr dptr, size_t length, unsigned int flags);
typedef CUresult CUDAAPI tcuStreamQuery(CUstream hStream);
typedef CUresult CUDAAPI tcuStreamSynchronize(CUstream hStream);
typedef CUresult CUDAAPI tcuStreamDestroy_v2(CUstream hStream);
typedef CUresult CUDAAPI tcuStreamCopyAttributes(CUstream dst, CUstream src);
typedef CUresult CUDAAPI tcuStreamGetAttribute(CUstream hStream, CUstreamAttrID attr, CUstreamAttrValue* value_out);
typedef CUresult CUDAAPI tcuStreamSetAttribute(CUstream hStream, CUstreamAttrID attr, const CUstreamAttrValue* value);
typedef CUresult CUDAAPI tcuEventCreate(CUevent* phEvent, unsigned int Flags);
typedef CUresult CUDAAPI tcuEventRecord(CUevent hEvent, CUstream hStream);
typedef CUresult CUDAAPI tcuEventRecordWithFlags(CUevent hEvent, CUstream hStream, unsigned int flags);
typedef CUresult CUDAAPI tcuEventQuery(CUevent hEvent);
typedef CUresult CUDAAPI tcuEventSynchronize(CUevent hEvent);
typedef CUresult CUDAAPI tcuEventDestroy_v2(CUevent hEvent);
typedef CUresult CUDAAPI tcuEventElapsedTime(float* pMilliseconds, CUevent hStart, CUevent hEnd);
typedef CUresult CUDAAPI tcuImportExternalMemory(CUexternalMemory* extMem_out, const CUDA_EXTERNAL_MEMORY_HANDLE_DESC* memHandleDesc);
typedef CUresult CUDAAPI tcuExternalMemoryGetMappedBuffer(CUdeviceptr* devPtr, CUexternalMemory extMem, const CUDA_EXTERNAL_MEMORY_BUFFER_DESC* bufferDesc);
typedef CUresult CUDAAPI tcuExternalMemoryGetMappedMipmappedArray(CUmipmappedArray* mipmap, CUexternalMemory extMem, const CUDA_EXTERNAL_MEMORY_MIPMAPPED_ARRAY_DESC* mipmapDesc);
typedef CUresult CUDAAPI tcuDestroyExternalMemory(CUexternalMemory extMem);
typedef CUresult CUDAAPI tcuImportExternalSemaphore(CUexternalSemaphore* extSem_out, const CUDA_EXTERNAL_SEMAPHORE_HANDLE_DESC* semHandleDesc);
typedef CUresult CUDAAPI tcuSignalExternalSemaphoresAsync(const CUexternalSemaphore* extSemArray, const CUDA_EXTERNAL_SEMAPHORE_SIGNAL_PARAMS* paramsArray, unsigned int numExtSems, CUstream stream);
typedef CUresult CUDAAPI tcuWaitExternalSemaphoresAsync(const CUexternalSemaphore* extSemArray, const CUDA_EXTERNAL_SEMAPHORE_WAIT_PARAMS* paramsArray, unsigned int numExtSems, CUstream stream);
typedef CUresult CUDAAPI tcuDestroyExternalSemaphore(CUexternalSemaphore extSem);
typedef CUresult CUDAAPI tcuStreamWaitValue32_v2(CUstream stream, CUdeviceptr addr, cuuint32_t value, unsigned int flags);
typedef CUresult CUDAAPI tcuStreamWaitValue64_v2(CUstream stream, CUdeviceptr addr, cuuint64_t value, unsigned int flags);
typedef CUresult CUDAAPI tcuStreamWriteValue32_v2(CUstream stream, CUdeviceptr addr, cuuint32_t value, unsigned int flags);
typedef CUresult CUDAAPI tcuStreamWriteValue64_v2(CUstream stream, CUdeviceptr addr, cuuint64_t value, unsigned int flags);
typedef CUresult CUDAAPI tcuStreamBatchMemOp_v2(CUstream stream, unsigned int count, CUstreamBatchMemOpParams* paramArray, unsigned int flags);
typedef CUresult CUDAAPI tcuFuncGetAttribute(int* pi, CUfunction_attribute attrib, CUfunction hfunc);
typedef CUresult CUDAAPI tcuFuncSetAttribute(CUfunction hfunc, CUfunction_attribute attrib, int value);
typedef CUresult CUDAAPI tcuFuncSetCacheConfig(CUfunction hfunc, CUfunc_cache config);
typedef CUresult CUDAAPI tcuFuncSetSharedMemConfig(CUfunction hfunc, CUsharedconfig config);
typedef CUresult CUDAAPI tcuFuncGetModule(CUmodule* hmod, CUfunction hfunc);
typedef CUresult CUDAAPI tcuLaunchKernel(CUfunction f, unsigned int gridDimX, unsigned int gridDimY, unsigned int gridDimZ, unsigned int blockDimX, unsigned int blockDimY, unsigned int blockDimZ, unsigned int sharedMemBytes, CUstream hStream, void** kernelParams, void** extra);
typedef CUresult CUDAAPI tcuLaunchKernelEx(const CUlaunchConfig* config, CUfunction f, void** kernelParams, void** extra);
typedef CUresult CUDAAPI tcuLaunchCooperativeKernel(CUfunction f, unsigned int gridDimX, unsigned int gridDimY, unsigned int gridDimZ, unsigned int blockDimX, unsigned int blockDimY, unsigned int blockDimZ, unsigned int sharedMemBytes, CUstream hStream, void** kernelParams);
typedef CUresult CUDAAPI tcuLaunchCooperativeKernelMultiDevice(CUDA_LAUNCH_PARAMS* launchParamsList, unsigned int numDevices, unsigned int flags);
typedef CUresult CUDAAPI tcuLaunchHostFunc(CUstream hStream, CUhostFn fn, void* userData);
typedef CUresult CUDAAPI tcuFuncSetBlockShape(CUfunction hfunc, int x, int y, int z);
typedef CUresult CUDAAPI tcuFuncSetSharedSize(CUfunction hfunc, unsigned int bytes);
typedef CUresult CUDAAPI tcuParamSetSize(CUfunction hfunc, unsigned int numbytes);
typedef CUresult CUDAAPI tcuParamSeti(CUfunction hfunc, int offset, unsigned int value);
typedef CUresult CUDAAPI tcuParamSetf(CUfunction hfunc, int offset, float value);
typedef CUresult CUDAAPI tcuParamSetv(CUfunction hfunc, int offset, void* ptr, unsigned int numbytes);
typedef CUresult CUDAAPI tcuLaunch(CUfunction f);
typedef CUresult CUDAAPI tcuLaunchGrid(CUfunction f, int grid_width, int grid_height);
typedef CUresult CUDAAPI tcuLaunchGridAsync(CUfunction f, int grid_width, int grid_height, CUstream hStream);
typedef CUresult CUDAAPI tcuParamSetTexRef(CUfunction hfunc, int texunit, CUtexref hTexRef);
typedef CUresult CUDAAPI tcuGraphCreate(CUgraph* phGraph, unsigned int flags);
typedef CUresult CUDAAPI tcuGraphAddKernelNode_v2(CUgraphNode* phGraphNode, CUgraph hGraph, const CUgraphNode* dependencies, size_t numDependencies, const CUDA_KERNEL_NODE_PARAMS* nodeParams);
typedef CUresult CUDAAPI tcuGraphKernelNodeGetParams_v2(CUgraphNode hNode, CUDA_KERNEL_NODE_PARAMS* nodeParams);
typedef CUresult CUDAAPI tcuGraphKernelNodeSetParams_v2(CUgraphNode hNode, const CUDA_KERNEL_NODE_PARAMS* nodeParams);
typedef CUresult CUDAAPI tcuGraphAddMemcpyNode(CUgraphNode* phGraphNode, CUgraph hGraph, const CUgraphNode* dependencies, size_t numDependencies, const CUDA_MEMCPY3D* copyParams, CUcontext ctx);
typedef CUresult CUDAAPI tcuGraphMemcpyNodeGetParams(CUgraphNode hNode, CUDA_MEMCPY3D* nodeParams);
typedef CUresult CUDAAPI tcuGraphMemcpyNodeSetParams(CUgraphNode hNode, const CUDA_MEMCPY3D* nodeParams);
typedef CUresult CUDAAPI tcuGraphAddMemsetNode(CUgraphNode* phGraphNode, CUgraph hGraph, const CUgraphNode* dependencies, size_t numDependencies, const CUDA_MEMSET_NODE_PARAMS* memsetParams, CUcontext ctx);
typedef CUresult CUDAAPI tcuGraphMemsetNodeGetParams(CUgraphNode hNode, CUDA_MEMSET_NODE_PARAMS* nodeParams);
typedef CUresult CUDAAPI tcuGraphMemsetNodeSetParams(CUgraphNode hNode, const CUDA_MEMSET_NODE_PARAMS* nodeParams);
typedef CUresult CUDAAPI tcuGraphAddHostNode(CUgraphNode* phGraphNode, CUgraph hGraph, const CUgraphNode* dependencies, size_t numDependencies, const CUDA_HOST_NODE_PARAMS* nodeParams);
typedef CUresult CUDAAPI tcuGraphHostNodeGetParams(CUgraphNode hNode, CUDA_HOST_NODE_PARAMS* nodeParams);
typedef CUresult CUDAAPI tcuGraphHostNodeSetParams(CUgraphNode hNode, const CUDA_HOST_NODE_PARAMS* nodeParams);
typedef CUresult CUDAAPI tcuGraphAddChildGraphNode(CUgraphNode* phGraphNode, CUgraph hGraph, const CUgraphNode* dependencies, size_t numDependencies, CUgraph childGraph);
typedef CUresult CUDAAPI tcuGraphChildGraphNodeGetGraph(CUgraphNode hNode, CUgraph* phGraph);
typedef CUresult CUDAAPI tcuGraphAddEmptyNode(CUgraphNode* phGraphNode, CUgraph hGraph, const CUgraphNode* dependencies, size_t numDependencies);
typedef CUresult CUDAAPI tcuGraphAddEventRecordNode(CUgraphNode* phGraphNode, CUgraph hGraph, const CUgraphNode* dependencies, size_t numDependencies, CUevent event);
typedef CUresult CUDAAPI tcuGraphEventRecordNodeGetEvent(CUgraphNode hNode, CUevent* event_out);
typedef CUresult CUDAAPI tcuGraphEventRecordNodeSetEvent(CUgraphNode hNode, CUevent event);
typedef CUresult CUDAAPI tcuGraphAddEventWaitNode(CUgraphNode* phGraphNode, CUgraph hGraph, const CUgraphNode* dependencies, size_t numDependencies, CUevent event);
typedef CUresult CUDAAPI tcuGraphEventWaitNodeGetEvent(CUgraphNode hNode, CUevent* event_out);
typedef CUresult CUDAAPI tcuGraphEventWaitNodeSetEvent(CUgraphNode hNode, CUevent event);
typedef CUresult CUDAAPI tcuGraphAddExternalSemaphoresSignalNode(CUgraphNode* phGraphNode, CUgraph hGraph, const CUgraphNode* dependencies, size_t numDependencies, const CUDA_EXT_SEM_SIGNAL_NODE_PARAMS* nodeParams);
typedef CUresult CUDAAPI tcuGraphExternalSemaphoresSignalNodeGetParams(CUgraphNode hNode, CUDA_EXT_SEM_SIGNAL_NODE_PARAMS* params_out);
typedef CUresult CUDAAPI tcuGraphExternalSemaphoresSignalNodeSetParams(CUgraphNode hNode, const CUDA_EXT_SEM_SIGNAL_NODE_PARAMS* nodeParams);
typedef CUresult CUDAAPI tcuGraphAddExternalSemaphoresWaitNode(CUgraphNode* phGraphNode, CUgraph hGraph, const CUgraphNode* dependencies, size_t numDependencies, const CUDA_EXT_SEM_WAIT_NODE_PARAMS* nodeParams);
typedef CUresult CUDAAPI tcuGraphExternalSemaphoresWaitNodeGetParams(CUgraphNode hNode, CUDA_EXT_SEM_WAIT_NODE_PARAMS* params_out);
typedef CUresult CUDAAPI tcuGraphExternalSemaphoresWaitNodeSetParams(CUgraphNode hNode, const CUDA_EXT_SEM_WAIT_NODE_PARAMS* nodeParams);
typedef CUresult CUDAAPI tcuGraphAddBatchMemOpNode(CUgraphNode* phGraphNode, CUgraph hGraph, const CUgraphNode* dependencies, size_t numDependencies, const CUDA_BATCH_MEM_OP_NODE_PARAMS* nodeParams);
typedef CUresult CUDAAPI tcuGraphBatchMemOpNodeGetParams(CUgraphNode hNode, CUDA_BATCH_MEM_OP_NODE_PARAMS* nodeParams_out);
typedef CUresult CUDAAPI tcuGraphBatchMemOpNodeSetParams(CUgraphNode hNode, const CUDA_BATCH_MEM_OP_NODE_PARAMS* nodeParams);
typedef CUresult CUDAAPI tcuGraphExecBatchMemOpNodeSetParams(CUgraphExec hGraphExec, CUgraphNode hNode, const CUDA_BATCH_MEM_OP_NODE_PARAMS* nodeParams);
typedef CUresult CUDAAPI tcuGraphAddMemAllocNode(CUgraphNode* phGraphNode, CUgraph hGraph, const CUgraphNode* dependencies, size_t numDependencies, CUDA_MEM_ALLOC_NODE_PARAMS* nodeParams);
typedef CUresult CUDAAPI tcuGraphMemAllocNodeGetParams(CUgraphNode hNode, CUDA_MEM_ALLOC_NODE_PARAMS* params_out);
typedef CUresult CUDAAPI tcuGraphAddMemFreeNode(CUgraphNode* phGraphNode, CUgraph hGraph, const CUgraphNode* dependencies, size_t numDependencies, CUdeviceptr dptr);
typedef CUresult CUDAAPI tcuGraphMemFreeNodeGetParams(CUgraphNode hNode, CUdeviceptr* dptr_out);
typedef CUresult CUDAAPI tcuDeviceGraphMemTrim(CUdevice device);
typedef CUresult CUDAAPI tcuDeviceGetGraphMemAttribute(CUdevice device, CUgraphMem_attribute attr, void* value);
typedef CUresult CUDAAPI tcuDeviceSetGraphMemAttribute(CUdevice device, CUgraphMem_attribute attr, void* value);
typedef CUresult CUDAAPI tcuGraphClone(CUgraph* phGraphClone, CUgraph originalGraph);
typedef CUresult CUDAAPI tcuGraphNodeFindInClone(CUgraphNode* phNode, CUgraphNode hOriginalNode, CUgraph hClonedGraph);
typedef CUresult CUDAAPI tcuGraphNodeGetType(CUgraphNode hNode, CUgraphNodeType* type);
typedef CUresult CUDAAPI tcuGraphGetNodes(CUgraph hGraph, CUgraphNode* nodes, size_t* numNodes);
typedef CUresult CUDAAPI tcuGraphGetRootNodes(CUgraph hGraph, CUgraphNode* rootNodes, size_t* numRootNodes);
typedef CUresult CUDAAPI tcuGraphGetEdges(CUgraph hGraph, CUgraphNode* from, CUgraphNode* to, size_t* numEdges);
typedef CUresult CUDAAPI tcuGraphNodeGetDependencies(CUgraphNode hNode, CUgraphNode* dependencies, size_t* numDependencies);
typedef CUresult CUDAAPI tcuGraphNodeGetDependentNodes(CUgraphNode hNode, CUgraphNode* dependentNodes, size_t* numDependentNodes);
typedef CUresult CUDAAPI tcuGraphAddDependencies(CUgraph hGraph, const CUgraphNode* from, const CUgraphNode* to, size_t numDependencies);
typedef CUresult CUDAAPI tcuGraphRemoveDependencies(CUgraph hGraph, const CUgraphNode* from, const CUgraphNode* to, size_t numDependencies);
typedef CUresult CUDAAPI tcuGraphDestroyNode(CUgraphNode hNode);
typedef CUresult CUDAAPI tcuGraphInstantiateWithFlags(CUgraphExec* phGraphExec, CUgraph hGraph, unsigned long long flags);
typedef CUresult CUDAAPI tcuGraphInstantiateWithParams(CUgraphExec* phGraphExec, CUgraph hGraph, CUDA_GRAPH_INSTANTIATE_PARAMS* instantiateParams);
typedef CUresult CUDAAPI tcuGraphExecGetFlags(CUgraphExec hGraphExec, cuuint64_t* flags);
typedef CUresult CUDAAPI tcuGraphExecKernelNodeSetParams_v2(CUgraphExec hGraphExec, CUgraphNode hNode, const CUDA_KERNEL_NODE_PARAMS* nodeParams);
typedef CUresult CUDAAPI tcuGraphExecMemcpyNodeSetParams(CUgraphExec hGraphExec, CUgraphNode hNode, const CUDA_MEMCPY3D* copyParams, CUcontext ctx);
typedef CUresult CUDAAPI tcuGraphExecMemsetNodeSetParams(CUgraphExec hGraphExec, CUgraphNode hNode, const CUDA_MEMSET_NODE_PARAMS* memsetParams, CUcontext ctx);
typedef CUresult CUDAAPI tcuGraphExecHostNodeSetParams(CUgraphExec hGraphExec, CUgraphNode hNode, const CUDA_HOST_NODE_PARAMS* nodeParams);
typedef CUresult CUDAAPI tcuGraphExecChildGraphNodeSetParams(CUgraphExec hGraphExec, CUgraphNode hNode, CUgraph childGraph);
typedef CUresult CUDAAPI tcuGraphExecEventRecordNodeSetEvent(CUgraphExec hGraphExec, CUgraphNode hNode, CUevent event);
typedef CUresult CUDAAPI tcuGraphExecEventWaitNodeSetEvent(CUgraphExec hGraphExec, CUgraphNode hNode, CUevent event);
typedef CUresult CUDAAPI tcuGraphExecExternalSemaphoresSignalNodeSetParams(CUgraphExec hGraphExec, CUgraphNode hNode, const CUDA_EXT_SEM_SIGNAL_NODE_PARAMS* nodeParams);
typedef CUresult CUDAAPI tcuGraphExecExternalSemaphoresWaitNodeSetParams(CUgraphExec hGraphExec, CUgraphNode hNode, const CUDA_EXT_SEM_WAIT_NODE_PARAMS* nodeParams);
typedef CUresult CUDAAPI tcuGraphNodeSetEnabled(CUgraphExec hGraphExec, CUgraphNode hNode, unsigned int isEnabled);
typedef CUresult CUDAAPI tcuGraphNodeGetEnabled(CUgraphExec hGraphExec, CUgraphNode hNode, unsigned int* isEnabled);
typedef CUresult CUDAAPI tcuGraphUpload(CUgraphExec hGraphExec, CUstream hStream);
typedef CUresult CUDAAPI tcuGraphLaunch(CUgraphExec hGraphExec, CUstream hStream);
typedef CUresult CUDAAPI tcuGraphExecDestroy(CUgraphExec hGraphExec);
typedef CUresult CUDAAPI tcuGraphDestroy(CUgraph hGraph);
typedef CUresult CUDAAPI tcuGraphExecUpdate_v2(CUgraphExec hGraphExec, CUgraph hGraph, CUgraphExecUpdateResultInfo* resultInfo);
typedef CUresult CUDAAPI tcuGraphKernelNodeCopyAttributes(CUgraphNode dst, CUgraphNode src);
typedef CUresult CUDAAPI tcuGraphKernelNodeGetAttribute(CUgraphNode hNode, CUkernelNodeAttrID attr, CUkernelNodeAttrValue* value_out);
typedef CUresult CUDAAPI tcuGraphKernelNodeSetAttribute(CUgraphNode hNode, CUkernelNodeAttrID attr, const CUkernelNodeAttrValue* value);
typedef CUresult CUDAAPI tcuGraphDebugDotPrint(CUgraph hGraph, const char* path, unsigned int flags);
typedef CUresult CUDAAPI tcuUserObjectCreate(CUuserObject* object_out, void* ptr, CUhostFn destroy, unsigned int initialRefcount, unsigned int flags);
typedef CUresult CUDAAPI tcuUserObjectRetain(CUuserObject object, unsigned int count);
typedef CUresult CUDAAPI tcuUserObjectRelease(CUuserObject object, unsigned int count);
typedef CUresult CUDAAPI tcuGraphRetainUserObject(CUgraph graph, CUuserObject object, unsigned int count, unsigned int flags);
typedef CUresult CUDAAPI tcuGraphReleaseUserObject(CUgraph graph, CUuserObject object, unsigned int count);
typedef CUresult CUDAAPI tcuOccupancyMaxActiveBlocksPerMultiprocessor(int* numBlocks, CUfunction func, int blockSize, size_t dynamicSMemSize);
typedef CUresult CUDAAPI tcuOccupancyMaxActiveBlocksPerMultiprocessorWithFlags(int* numBlocks, CUfunction func, int blockSize, size_t dynamicSMemSize, unsigned int flags);
typedef CUresult CUDAAPI tcuOccupancyMaxPotentialBlockSize(int* minGridSize, int* blockSize, CUfunction func, CUoccupancyB2DSize blockSizeToDynamicSMemSize, size_t dynamicSMemSize, int blockSizeLimit);
typedef CUresult CUDAAPI tcuOccupancyMaxPotentialBlockSizeWithFlags(int* minGridSize, int* blockSize, CUfunction func, CUoccupancyB2DSize blockSizeToDynamicSMemSize, size_t dynamicSMemSize, int blockSizeLimit, unsigned int flags);
typedef CUresult CUDAAPI tcuOccupancyAvailableDynamicSMemPerBlock(size_t* dynamicSmemSize, CUfunction func, int numBlocks, int blockSize);
typedef CUresult CUDAAPI tcuOccupancyMaxPotentialClusterSize(int* clusterSize, CUfunction func, const CUlaunchConfig* config);
typedef CUresult CUDAAPI tcuOccupancyMaxActiveClusters(int* numClusters, CUfunction func, const CUlaunchConfig* config);
typedef CUresult CUDAAPI tcuTexRefSetArray(CUtexref hTexRef, CUarray hArray, unsigned int Flags);
typedef CUresult CUDAAPI tcuTexRefSetMipmappedArray(CUtexref hTexRef, CUmipmappedArray hMipmappedArray, unsigned int Flags);
typedef CUresult CUDAAPI tcuTexRefSetAddress_v2(size_t* ByteOffset, CUtexref hTexRef, CUdeviceptr dptr, size_t bytes);
typedef CUresult CUDAAPI tcuTexRefSetAddress2D_v3(CUtexref hTexRef, const CUDA_ARRAY_DESCRIPTOR* desc, CUdeviceptr dptr, size_t Pitch);
typedef CUresult CUDAAPI tcuTexRefSetFormat(CUtexref hTexRef, CUarray_format fmt, int NumPackedComponents);
typedef CUresult CUDAAPI tcuTexRefSetAddressMode(CUtexref hTexRef, int dim, CUaddress_mode am);
typedef CUresult CUDAAPI tcuTexRefSetFilterMode(CUtexref hTexRef, CUfilter_mode fm);
typedef CUresult CUDAAPI tcuTexRefSetMipmapFilterMode(CUtexref hTexRef, CUfilter_mode fm);
typedef CUresult CUDAAPI tcuTexRefSetMipmapLevelBias(CUtexref hTexRef, float bias);
typedef CUresult CUDAAPI tcuTexRefSetMipmapLevelClamp(CUtexref hTexRef, float minMipmapLevelClamp, float maxMipmapLevelClamp);
typedef CUresult CUDAAPI tcuTexRefSetMaxAnisotropy(CUtexref hTexRef, unsigned int maxAniso);
typedef CUresult CUDAAPI tcuTexRefSetBorderColor(CUtexref hTexRef, float* pBorderColor);
typedef CUresult CUDAAPI tcuTexRefSetFlags(CUtexref hTexRef, unsigned int Flags);
typedef CUresult CUDAAPI tcuTexRefGetAddress_v2(CUdeviceptr* pdptr, CUtexref hTexRef);
typedef CUresult CUDAAPI tcuTexRefGetArray(CUarray* phArray, CUtexref hTexRef);
typedef CUresult CUDAAPI tcuTexRefGetMipmappedArray(CUmipmappedArray* phMipmappedArray, CUtexref hTexRef);
typedef CUresult CUDAAPI tcuTexRefGetAddressMode(CUaddress_mode* pam, CUtexref hTexRef, int dim);
typedef CUresult CUDAAPI tcuTexRefGetFilterMode(CUfilter_mode* pfm, CUtexref hTexRef);
typedef CUresult CUDAAPI tcuTexRefGetFormat(CUarray_format* pFormat, int* pNumChannels, CUtexref hTexRef);
typedef CUresult CUDAAPI tcuTexRefGetMipmapFilterMode(CUfilter_mode* pfm, CUtexref hTexRef);
typedef CUresult CUDAAPI tcuTexRefGetMipmapLevelBias(float* pbias, CUtexref hTexRef);
typedef CUresult CUDAAPI tcuTexRefGetMipmapLevelClamp(float* pminMipmapLevelClamp, float* pmaxMipmapLevelClamp, CUtexref hTexRef);
typedef CUresult CUDAAPI tcuTexRefGetMaxAnisotropy(int* pmaxAniso, CUtexref hTexRef);
typedef CUresult CUDAAPI tcuTexRefGetBorderColor(float* pBorderColor, CUtexref hTexRef);
typedef CUresult CUDAAPI tcuTexRefGetFlags(unsigned int* pFlags, CUtexref hTexRef);
typedef CUresult CUDAAPI tcuTexRefCreate(CUtexref* pTexRef);
typedef CUresult CUDAAPI tcuTexRefDestroy(CUtexref hTexRef);
typedef CUresult CUDAAPI tcuSurfRefSetArray(CUsurfref hSurfRef, CUarray hArray, unsigned int Flags);
typedef CUresult CUDAAPI tcuSurfRefGetArray(CUarray* phArray, CUsurfref hSurfRef);
typedef CUresult CUDAAPI tcuTexObjectCreate(CUtexObject* pTexObject, const CUDA_RESOURCE_DESC* pResDesc, const CUDA_TEXTURE_DESC* pTexDesc, const CUDA_RESOURCE_VIEW_DESC* pResViewDesc);
typedef CUresult CUDAAPI tcuTexObjectDestroy(CUtexObject texObject);
typedef CUresult CUDAAPI tcuTexObjectGetResourceDesc(CUDA_RESOURCE_DESC* pResDesc, CUtexObject texObject);
typedef CUresult CUDAAPI tcuTexObjectGetTextureDesc(CUDA_TEXTURE_DESC* pTexDesc, CUtexObject texObject);
typedef CUresult CUDAAPI tcuTexObjectGetResourceViewDesc(CUDA_RESOURCE_VIEW_DESC* pResViewDesc, CUtexObject texObject);
typedef CUresult CUDAAPI tcuSurfObjectCreate(CUsurfObject* pSurfObject, const CUDA_RESOURCE_DESC* pResDesc);
typedef CUresult CUDAAPI tcuSurfObjectDestroy(CUsurfObject surfObject);
typedef CUresult CUDAAPI tcuSurfObjectGetResourceDesc(CUDA_RESOURCE_DESC* pResDesc, CUsurfObject surfObject);
typedef CUresult CUDAAPI tcuTensorMapEncodeTiled(CUtensorMap* tensorMap, CUtensorMapDataType tensorDataType, cuuint32_t tensorRank, void* globalAddress, const cuuint64_t* globalDim, const cuuint64_t* globalStrides, const cuuint32_t* boxDim, const cuuint32_t* elementStrides, CUtensorMapInterleave interleave, CUtensorMapSwizzle swizzle, CUtensorMapL2promotion l2Promotion, CUtensorMapFloatOOBfill oobFill);
typedef CUresult CUDAAPI tcuTensorMapEncodeIm2col(CUtensorMap* tensorMap, CUtensorMapDataType tensorDataType, cuuint32_t tensorRank, void* globalAddress, const cuuint64_t* globalDim, const cuuint64_t* globalStrides, const int* pixelBoxLowerCorner, const int* pixelBoxUpperCorner, cuuint32_t channelsPerPixel, cuuint32_t pixelsPerColumn, const cuuint32_t* elementStrides, CUtensorMapInterleave interleave, CUtensorMapSwizzle swizzle, CUtensorMapL2promotion l2Promotion, CUtensorMapFloatOOBfill oobFill);
typedef CUresult CUDAAPI tcuTensorMapReplaceAddress(CUtensorMap* tensorMap, void* globalAddress);
typedef CUresult CUDAAPI tcuDeviceCanAccessPeer(int* canAccessPeer, CUdevice dev, CUdevice peerDev);
typedef CUresult CUDAAPI tcuCtxEnablePeerAccess(CUcontext peerContext, unsigned int Flags);
typedef CUresult CUDAAPI tcuCtxDisablePeerAccess(CUcontext peerContext);
typedef CUresult CUDAAPI tcuDeviceGetP2PAttribute(int* value, CUdevice_P2PAttribute attrib, CUdevice srcDevice, CUdevice dstDevice);
typedef CUresult CUDAAPI tcuGraphicsUnregisterResource(CUgraphicsResource resource);
typedef CUresult CUDAAPI tcuGraphicsSubResourceGetMappedArray(CUarray* pArray, CUgraphicsResource resource, unsigned int arrayIndex, unsigned int mipLevel);
typedef CUresult CUDAAPI tcuGraphicsResourceGetMappedMipmappedArray(CUmipmappedArray* pMipmappedArray, CUgraphicsResource resource);
typedef CUresult CUDAAPI tcuGraphicsResourceGetMappedPointer_v2(CUdeviceptr* pDevPtr, size_t* pSize, CUgraphicsResource resource);
typedef CUresult CUDAAPI tcuGraphicsResourceSetMapFlags_v2(CUgraphicsResource resource, unsigned int flags);
typedef CUresult CUDAAPI tcuGraphicsMapResources(unsigned int count, CUgraphicsResource* resources, CUstream hStream);
typedef CUresult CUDAAPI tcuGraphicsUnmapResources(unsigned int count, CUgraphicsResource* resources, CUstream hStream);
typedef CUresult CUDAAPI tcuGetProcAddress_v2(const char* symbol, void** pfn, int cudaVersion, cuuint64_t flags, CUdriverProcAddressQueryResult* symbolStatus);
typedef CUresult CUDAAPI tcuCoredumpGetAttribute(CUcoredumpSettings attrib, void* value, size_t* size);
typedef CUresult CUDAAPI tcuCoredumpGetAttributeGlobal(CUcoredumpSettings attrib, void* value, size_t* size);
typedef CUresult CUDAAPI tcuCoredumpSetAttribute(CUcoredumpSettings attrib, void* value, size_t* size);
typedef CUresult CUDAAPI tcuCoredumpSetAttributeGlobal(CUcoredumpSettings attrib, void* value, size_t* size);
typedef CUresult CUDAAPI tcuGetExportTable(const void** ppExportTable, const CUuuid* pExportTableId);

typedef const char* CUDAAPI tnvrtcGetErrorString(nvrtcResult result);
typedef nvrtcResult CUDAAPI tnvrtcVersion(int* major, int* minor);
typedef nvrtcResult CUDAAPI tnvrtcGetNumSupportedArchs(int* numArchs);
typedef nvrtcResult CUDAAPI tnvrtcGetSupportedArchs(int* supportedArchs);
typedef nvrtcResult CUDAAPI tnvrtcCreateProgram(nvrtcProgram* prog, const char* src, const char* name, int numHeaders, const char** headers, const char** includeNames);
typedef nvrtcResult CUDAAPI tnvrtcDestroyProgram(nvrtcProgram* prog);
typedef nvrtcResult CUDAAPI tnvrtcCompileProgram(nvrtcProgram prog, int numOptions, const char** options);
typedef nvrtcResult CUDAAPI tnvrtcGetPTXSize(nvrtcProgram prog, size_t* ptxSizeRet);
typedef nvrtcResult CUDAAPI tnvrtcGetPTX(nvrtcProgram prog, char* ptx);
typedef nvrtcResult CUDAAPI tnvrtcGetCUBINSize(nvrtcProgram prog, size_t* cubinSizeRet);
typedef nvrtcResult CUDAAPI tnvrtcGetCUBIN(nvrtcProgram prog, char* cubin);
typedef nvrtcResult CUDAAPI tnvrtcGetNVVMSize(nvrtcProgram prog, size_t* nvvmSizeRet);
typedef nvrtcResult CUDAAPI tnvrtcGetNVVM(nvrtcProgram prog, char* nvvm);
typedef nvrtcResult CUDAAPI tnvrtcGetLTOIRSize(nvrtcProgram prog, size_t* LTOIRSizeRet);
typedef nvrtcResult CUDAAPI tnvrtcGetLTOIR(nvrtcProgram prog, char* LTOIR);
typedef nvrtcResult CUDAAPI tnvrtcGetOptiXIRSize(nvrtcProgram prog, size_t* optixirSizeRet);
typedef nvrtcResult CUDAAPI tnvrtcGetOptiXIR(nvrtcProgram prog, char* optixir);
typedef nvrtcResult CUDAAPI tnvrtcGetProgramLogSize(nvrtcProgram prog, size_t* logSizeRet);
typedef nvrtcResult CUDAAPI tnvrtcGetProgramLog(nvrtcProgram prog, char* log);
typedef nvrtcResult CUDAAPI tnvrtcAddNameExpression(nvrtcProgram prog, const char* name_expression);
typedef nvrtcResult CUDAAPI tnvrtcGetLoweredName(nvrtcProgram prog, const char* name_expression, const char** lowered_name);

typedef size_t CUDAAPI tcudnnGetVersion(void);
typedef size_t CUDAAPI tcudnnGetMaxDeviceVersion(void);
typedef size_t CUDAAPI tcudnnGetCudartVersion(void);
typedef const char* CUDAAPI tcudnnGetErrorString(cudnnStatus_t status);
typedef cudnnStatus_t CUDAAPI tcudnnQueryRuntimeError(cudnnHandle_t handle, cudnnStatus_t* rstatus, cudnnErrQueryMode_t mode, cudnnRuntimeTag_t* tag);
typedef cudnnStatus_t CUDAAPI tcudnnGetProperty(libraryPropertyType type, int* value);
typedef cudnnStatus_t CUDAAPI tcudnnCreate(cudnnHandle_t* handle);
typedef cudnnStatus_t CUDAAPI tcudnnDestroy(cudnnHandle_t handle);
typedef cudnnStatus_t CUDAAPI tcudnnSetStream(cudnnHandle_t handle, cudaStream_t streamId);
typedef cudnnStatus_t CUDAAPI tcudnnGetStream(cudnnHandle_t handle, cudaStream_t* streamId);
typedef cudnnStatus_t CUDAAPI tcudnnCreateTensorDescriptor(cudnnTensorDescriptor_t* tensorDesc);
typedef cudnnStatus_t CUDAAPI tcudnnSetTensor4dDescriptor(cudnnTensorDescriptor_t tensorDesc, cudnnTensorFormat_t format, cudnnDataType_t dataType, int n, int c, int h, int w);
typedef cudnnStatus_t CUDAAPI tcudnnSetTensor4dDescriptorEx(cudnnTensorDescriptor_t tensorDesc, cudnnDataType_t dataType, int n, int c, int h, int w, int nStride, int cStride, int hStride, int wStride);
typedef cudnnStatus_t CUDAAPI tcudnnGetTensor4dDescriptor(const cudnnTensorDescriptor_t tensorDesc, cudnnDataType_t* dataType, int* n, int* c, int* h, int* w, int* nStride, int* cStride, int* hStride, int* wStride);
typedef cudnnStatus_t CUDAAPI tcudnnSetTensorNdDescriptor(cudnnTensorDescriptor_t tensorDesc, cudnnDataType_t dataType, int nbDims, const int dimA[], const int strideA[]);
typedef cudnnStatus_t CUDAAPI tcudnnSetTensorNdDescriptorEx(cudnnTensorDescriptor_t tensorDesc, cudnnTensorFormat_t format, cudnnDataType_t dataType, int nbDims, const int dimA[]);
typedef cudnnStatus_t CUDAAPI tcudnnGetTensorNdDescriptor(const cudnnTensorDescriptor_t tensorDesc, int nbDimsRequested, cudnnDataType_t* dataType, int* nbDims, int dimA[], int strideA[]);
typedef cudnnStatus_t CUDAAPI tcudnnGetTensorSizeInBytes(const cudnnTensorDescriptor_t tensorDesc, size_t* size);
typedef cudnnStatus_t CUDAAPI tcudnnDestroyTensorDescriptor(cudnnTensorDescriptor_t tensorDesc);
typedef cudnnStatus_t CUDAAPI tcudnnInitTransformDest(const cudnnTensorTransformDescriptor_t transformDesc, const cudnnTensorDescriptor_t srcDesc, cudnnTensorDescriptor_t destDesc, size_t* destSizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnCreateTensorTransformDescriptor(cudnnTensorTransformDescriptor_t* transformDesc);
typedef cudnnStatus_t CUDAAPI tcudnnSetTensorTransformDescriptor(cudnnTensorTransformDescriptor_t transformDesc, const uint32_t nbDims, const cudnnTensorFormat_t destFormat, const int32_t padBeforeA[], const int32_t padAfterA[], const uint32_t foldA[], const cudnnFoldingDirection_t direction);
typedef cudnnStatus_t CUDAAPI tcudnnGetTensorTransformDescriptor(cudnnTensorTransformDescriptor_t transformDesc, uint32_t nbDimsRequested, cudnnTensorFormat_t* destFormat, int32_t padBeforeA[], int32_t padAfterA[], uint32_t foldA[], cudnnFoldingDirection_t* direction);
typedef cudnnStatus_t CUDAAPI tcudnnDestroyTensorTransformDescriptor(cudnnTensorTransformDescriptor_t transformDesc);
typedef cudnnStatus_t CUDAAPI tcudnnTransformTensor(cudnnHandle_t handle, const void* alpha, const cudnnTensorDescriptor_t xDesc, const void* x, const void* beta, const cudnnTensorDescriptor_t yDesc, void* y);
typedef cudnnStatus_t CUDAAPI tcudnnTransformTensorEx(cudnnHandle_t handle, const cudnnTensorTransformDescriptor_t transDesc, const void* alpha, const cudnnTensorDescriptor_t srcDesc, const void* srcData, const void* beta, const cudnnTensorDescriptor_t destDesc, void* destData);
typedef cudnnStatus_t CUDAAPI tcudnnAddTensor(cudnnHandle_t handle, const void* alpha, const cudnnTensorDescriptor_t aDesc, const void* A, const void* beta, const cudnnTensorDescriptor_t cDesc, void* C);
typedef cudnnStatus_t CUDAAPI tcudnnCreateOpTensorDescriptor(cudnnOpTensorDescriptor_t* opTensorDesc);
typedef cudnnStatus_t CUDAAPI tcudnnSetOpTensorDescriptor(cudnnOpTensorDescriptor_t opTensorDesc, cudnnOpTensorOp_t opTensorOp, cudnnDataType_t opTensorCompType, cudnnNanPropagation_t opTensorNanOpt);
typedef cudnnStatus_t CUDAAPI tcudnnGetOpTensorDescriptor(const cudnnOpTensorDescriptor_t opTensorDesc, cudnnOpTensorOp_t* opTensorOp, cudnnDataType_t* opTensorCompType, cudnnNanPropagation_t* opTensorNanOpt);
typedef cudnnStatus_t CUDAAPI tcudnnDestroyOpTensorDescriptor(cudnnOpTensorDescriptor_t opTensorDesc);
typedef cudnnStatus_t CUDAAPI tcudnnOpTensor(cudnnHandle_t handle, const cudnnOpTensorDescriptor_t opTensorDesc, const void* alpha1, const cudnnTensorDescriptor_t aDesc, const void* A, const void* alpha2, const cudnnTensorDescriptor_t bDesc, const void* B, const void* beta, const cudnnTensorDescriptor_t cDesc, void* C);
typedef cudnnStatus_t CUDAAPI tcudnnCreateReduceTensorDescriptor(cudnnReduceTensorDescriptor_t* reduceTensorDesc);
typedef cudnnStatus_t CUDAAPI tcudnnSetReduceTensorDescriptor(cudnnReduceTensorDescriptor_t reduceTensorDesc, cudnnReduceTensorOp_t reduceTensorOp, cudnnDataType_t reduceTensorCompType, cudnnNanPropagation_t reduceTensorNanOpt, cudnnReduceTensorIndices_t reduceTensorIndices, cudnnIndicesType_t reduceTensorIndicesType);
typedef cudnnStatus_t CUDAAPI tcudnnGetReduceTensorDescriptor(const cudnnReduceTensorDescriptor_t reduceTensorDesc, cudnnReduceTensorOp_t* reduceTensorOp, cudnnDataType_t* reduceTensorCompType, cudnnNanPropagation_t* reduceTensorNanOpt, cudnnReduceTensorIndices_t* reduceTensorIndices, cudnnIndicesType_t* reduceTensorIndicesType);
typedef cudnnStatus_t CUDAAPI tcudnnDestroyReduceTensorDescriptor(cudnnReduceTensorDescriptor_t reduceTensorDesc);
typedef cudnnStatus_t CUDAAPI tcudnnGetReductionIndicesSize(cudnnHandle_t handle, const cudnnReduceTensorDescriptor_t reduceTensorDesc, const cudnnTensorDescriptor_t aDesc, const cudnnTensorDescriptor_t cDesc, size_t* sizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnGetReductionWorkspaceSize(cudnnHandle_t handle, const cudnnReduceTensorDescriptor_t reduceTensorDesc, const cudnnTensorDescriptor_t aDesc, const cudnnTensorDescriptor_t cDesc, size_t* sizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnReduceTensor(cudnnHandle_t handle, const cudnnReduceTensorDescriptor_t reduceTensorDesc, void* indices, size_t indicesSizeInBytes, void* workspace, size_t workspaceSizeInBytes, const void* alpha, const cudnnTensorDescriptor_t aDesc, const void* A, const void* beta, const cudnnTensorDescriptor_t cDesc, void* C);
typedef cudnnStatus_t CUDAAPI tcudnnSetTensor(cudnnHandle_t handle, const cudnnTensorDescriptor_t yDesc, void* y, const void* valuePtr);
typedef cudnnStatus_t CUDAAPI tcudnnScaleTensor(cudnnHandle_t handle, const cudnnTensorDescriptor_t yDesc, void* y, const void* alpha);
typedef cudnnStatus_t CUDAAPI tcudnnCreateFilterDescriptor(cudnnFilterDescriptor_t* filterDesc);
typedef cudnnStatus_t CUDAAPI tcudnnSetFilter4dDescriptor(cudnnFilterDescriptor_t filterDesc, cudnnDataType_t dataType, cudnnTensorFormat_t format, int k, int c, int h, int w);
typedef cudnnStatus_t CUDAAPI tcudnnGetFilter4dDescriptor(const cudnnFilterDescriptor_t filterDesc, cudnnDataType_t* dataType, cudnnTensorFormat_t* format, int* k, int* c, int* h, int* w);
typedef cudnnStatus_t CUDAAPI tcudnnSetFilterNdDescriptor(cudnnFilterDescriptor_t filterDesc, cudnnDataType_t dataType, cudnnTensorFormat_t format, int nbDims, const int filterDimA[]);
typedef cudnnStatus_t CUDAAPI tcudnnGetFilterNdDescriptor(const cudnnFilterDescriptor_t filterDesc, int nbDimsRequested, cudnnDataType_t* dataType, cudnnTensorFormat_t* format, int* nbDims, int filterDimA[]);
typedef cudnnStatus_t CUDAAPI tcudnnGetFilterSizeInBytes(const cudnnFilterDescriptor_t filterDesc, size_t* size);
typedef cudnnStatus_t CUDAAPI tcudnnTransformFilter(cudnnHandle_t handle, const cudnnTensorTransformDescriptor_t transDesc, const void* alpha, const cudnnFilterDescriptor_t srcDesc, const void* srcData, const void* beta, const cudnnFilterDescriptor_t destDesc, void* destData);
typedef cudnnStatus_t CUDAAPI tcudnnDestroyFilterDescriptor(cudnnFilterDescriptor_t filterDesc);
typedef cudnnStatus_t CUDAAPI tcudnnSoftmaxForward(cudnnHandle_t handle, cudnnSoftmaxAlgorithm_t algo, cudnnSoftmaxMode_t mode, const void* alpha, const cudnnTensorDescriptor_t xDesc, const void* x, const void* beta, const cudnnTensorDescriptor_t yDesc, void* y);
typedef cudnnStatus_t CUDAAPI tcudnnCreatePoolingDescriptor(cudnnPoolingDescriptor_t* poolingDesc);
typedef cudnnStatus_t CUDAAPI tcudnnSetPooling2dDescriptor(cudnnPoolingDescriptor_t poolingDesc, cudnnPoolingMode_t mode, cudnnNanPropagation_t maxpoolingNanOpt, int windowHeight, int windowWidth, int verticalPadding, int horizontalPadding, int verticalStride, int horizontalStride);
typedef cudnnStatus_t CUDAAPI tcudnnGetPooling2dDescriptor(const cudnnPoolingDescriptor_t poolingDesc, cudnnPoolingMode_t* mode, cudnnNanPropagation_t* maxpoolingNanOpt, int* windowHeight, int* windowWidth, int* verticalPadding, int* horizontalPadding, int* verticalStride, int* horizontalStride);
typedef cudnnStatus_t CUDAAPI tcudnnSetPoolingNdDescriptor(cudnnPoolingDescriptor_t poolingDesc, const cudnnPoolingMode_t mode, const cudnnNanPropagation_t maxpoolingNanOpt, int nbDims, const int windowDimA[], const int paddingA[], const int strideA[]);
typedef cudnnStatus_t CUDAAPI tcudnnGetPoolingNdDescriptor(const cudnnPoolingDescriptor_t poolingDesc, int nbDimsRequested, cudnnPoolingMode_t* mode, cudnnNanPropagation_t* maxpoolingNanOpt, int* nbDims, int windowDimA[], int paddingA[], int strideA[]);
typedef cudnnStatus_t CUDAAPI tcudnnGetPoolingNdForwardOutputDim(const cudnnPoolingDescriptor_t poolingDesc, const cudnnTensorDescriptor_t inputTensorDesc, int nbDims, int outputTensorDimA[]);
typedef cudnnStatus_t CUDAAPI tcudnnGetPooling2dForwardOutputDim(const cudnnPoolingDescriptor_t poolingDesc, const cudnnTensorDescriptor_t inputTensorDesc, int* n, int* c, int* h, int* w);
typedef cudnnStatus_t CUDAAPI tcudnnDestroyPoolingDescriptor(cudnnPoolingDescriptor_t poolingDesc);
typedef cudnnStatus_t CUDAAPI tcudnnPoolingForward(cudnnHandle_t handle, const cudnnPoolingDescriptor_t poolingDesc, const void* alpha, const cudnnTensorDescriptor_t xDesc, const void* x, const void* beta, const cudnnTensorDescriptor_t yDesc, void* y);
typedef cudnnStatus_t CUDAAPI tcudnnCreateActivationDescriptor(cudnnActivationDescriptor_t* activationDesc);
typedef cudnnStatus_t CUDAAPI tcudnnSetActivationDescriptor(cudnnActivationDescriptor_t activationDesc, cudnnActivationMode_t mode, cudnnNanPropagation_t reluNanOpt, double coef);
typedef cudnnStatus_t CUDAAPI tcudnnGetActivationDescriptor(const cudnnActivationDescriptor_t activationDesc, cudnnActivationMode_t* mode, cudnnNanPropagation_t* reluNanOpt, double* coef);
typedef cudnnStatus_t CUDAAPI tcudnnSetActivationDescriptorSwishBeta(cudnnActivationDescriptor_t activationDesc, double swish_beta);
typedef cudnnStatus_t CUDAAPI tcudnnGetActivationDescriptorSwishBeta(cudnnActivationDescriptor_t activationDesc, double* swish_beta);
typedef cudnnStatus_t CUDAAPI tcudnnDestroyActivationDescriptor(cudnnActivationDescriptor_t activationDesc);
typedef cudnnStatus_t CUDAAPI tcudnnActivationForward(cudnnHandle_t handle, cudnnActivationDescriptor_t activationDesc, const void* alpha, const cudnnTensorDescriptor_t xDesc, const void* x, const void* beta, const cudnnTensorDescriptor_t yDesc, void* y);
typedef cudnnStatus_t CUDAAPI tcudnnCreateLRNDescriptor(cudnnLRNDescriptor_t* normDesc);
typedef cudnnStatus_t CUDAAPI tcudnnSetLRNDescriptor(cudnnLRNDescriptor_t normDesc, unsigned lrnN, double lrnAlpha, double lrnBeta, double lrnK);
typedef cudnnStatus_t CUDAAPI tcudnnGetLRNDescriptor(cudnnLRNDescriptor_t normDesc, unsigned* lrnN, double* lrnAlpha, double* lrnBeta, double* lrnK);
typedef cudnnStatus_t CUDAAPI tcudnnDestroyLRNDescriptor(cudnnLRNDescriptor_t lrnDesc);
typedef cudnnStatus_t CUDAAPI tcudnnLRNCrossChannelForward(cudnnHandle_t handle, cudnnLRNDescriptor_t normDesc, cudnnLRNMode_t lrnMode, const void* alpha, const cudnnTensorDescriptor_t xDesc, const void* x, const void* beta, const cudnnTensorDescriptor_t yDesc, void* y);
typedef cudnnStatus_t CUDAAPI tcudnnDivisiveNormalizationForward(cudnnHandle_t handle, cudnnLRNDescriptor_t normDesc, cudnnDivNormMode_t mode, const void* alpha, const cudnnTensorDescriptor_t xDesc, const void* x, const void* means, void* temp, void* temp2, const void* beta, const cudnnTensorDescriptor_t yDesc, void* y);
typedef cudnnStatus_t CUDAAPI tcudnnDeriveBNTensorDescriptor(cudnnTensorDescriptor_t derivedBnDesc, const cudnnTensorDescriptor_t xDesc, cudnnBatchNormMode_t mode);
typedef cudnnStatus_t CUDAAPI tcudnnBatchNormalizationForwardInference(cudnnHandle_t handle, cudnnBatchNormMode_t mode, const void* alpha, const void* beta, const cudnnTensorDescriptor_t xDesc, const void* x, const cudnnTensorDescriptor_t yDesc, void* y, const cudnnTensorDescriptor_t bnScaleBiasMeanVarDesc, const void* bnScale, const void* bnBias, const void* estimatedMean, const void* estimatedVariance, double epsilon);
typedef cudnnStatus_t CUDAAPI tcudnnDeriveNormTensorDescriptor(cudnnTensorDescriptor_t derivedNormScaleBiasDesc, cudnnTensorDescriptor_t derivedNormMeanVarDesc, const cudnnTensorDescriptor_t xDesc, cudnnNormMode_t mode, int groupCnt);
typedef cudnnStatus_t CUDAAPI tcudnnNormalizationForwardInference(cudnnHandle_t handle, cudnnNormMode_t mode, cudnnNormOps_t normOps, cudnnNormAlgo_t algo, const void* alpha, const void* beta, const cudnnTensorDescriptor_t xDesc, const void* x, const cudnnTensorDescriptor_t normScaleBiasDesc, const void* normScale, const void* normBias, const cudnnTensorDescriptor_t normMeanVarDesc, const void* estimatedMean, const void* estimatedVariance, const cudnnTensorDescriptor_t zDesc, const void* z, cudnnActivationDescriptor_t activationDesc, const cudnnTensorDescriptor_t yDesc, void* y, double epsilon, int groupCnt);
typedef cudnnStatus_t CUDAAPI tcudnnCreateSpatialTransformerDescriptor(cudnnSpatialTransformerDescriptor_t* stDesc);
typedef cudnnStatus_t CUDAAPI tcudnnSetSpatialTransformerNdDescriptor(cudnnSpatialTransformerDescriptor_t stDesc, cudnnSamplerType_t samplerType, cudnnDataType_t dataType, const int nbDims, const int dimA[]);
typedef cudnnStatus_t CUDAAPI tcudnnDestroySpatialTransformerDescriptor(cudnnSpatialTransformerDescriptor_t stDesc);
typedef cudnnStatus_t CUDAAPI tcudnnSpatialTfGridGeneratorForward(cudnnHandle_t handle, const cudnnSpatialTransformerDescriptor_t stDesc, const void* theta, void* grid);
typedef cudnnStatus_t CUDAAPI tcudnnSpatialTfSamplerForward(cudnnHandle_t handle, cudnnSpatialTransformerDescriptor_t stDesc, const void* alpha, const cudnnTensorDescriptor_t xDesc, const void* x, const void* grid, const void* beta, cudnnTensorDescriptor_t yDesc, void* y);
typedef cudnnStatus_t CUDAAPI tcudnnCreateDropoutDescriptor(cudnnDropoutDescriptor_t* dropoutDesc);
typedef cudnnStatus_t CUDAAPI tcudnnDestroyDropoutDescriptor(cudnnDropoutDescriptor_t dropoutDesc);
typedef cudnnStatus_t CUDAAPI tcudnnDropoutGetStatesSize(cudnnHandle_t handle, size_t* sizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnDropoutGetReserveSpaceSize(cudnnTensorDescriptor_t xdesc, size_t* sizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnSetDropoutDescriptor(cudnnDropoutDescriptor_t dropoutDesc, cudnnHandle_t handle, float dropout, void* states, size_t stateSizeInBytes, unsigned long long seed);
typedef cudnnStatus_t CUDAAPI tcudnnRestoreDropoutDescriptor(cudnnDropoutDescriptor_t dropoutDesc, cudnnHandle_t handle, float dropout, void* states, size_t stateSizeInBytes, unsigned long long seed);
typedef cudnnStatus_t CUDAAPI tcudnnGetDropoutDescriptor(cudnnDropoutDescriptor_t dropoutDesc, cudnnHandle_t handle, float* dropout, void** states, unsigned long long* seed);
typedef cudnnStatus_t CUDAAPI tcudnnDropoutForward(cudnnHandle_t handle, const cudnnDropoutDescriptor_t dropoutDesc, const cudnnTensorDescriptor_t xdesc, const void* x, const cudnnTensorDescriptor_t ydesc, void* y, void* reserveSpace, size_t reserveSpaceSizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnCreateAlgorithmDescriptor(cudnnAlgorithmDescriptor_t* algoDesc);
typedef cudnnStatus_t CUDAAPI tcudnnSetAlgorithmDescriptor(cudnnAlgorithmDescriptor_t algoDesc, cudnnAlgorithm_t algorithm);
typedef cudnnStatus_t CUDAAPI tcudnnGetAlgorithmDescriptor(const cudnnAlgorithmDescriptor_t algoDesc, cudnnAlgorithm_t* algorithm);
typedef cudnnStatus_t CUDAAPI tcudnnCopyAlgorithmDescriptor(const cudnnAlgorithmDescriptor_t src, cudnnAlgorithmDescriptor_t dest);
typedef cudnnStatus_t CUDAAPI tcudnnDestroyAlgorithmDescriptor(cudnnAlgorithmDescriptor_t algoDesc);
typedef cudnnStatus_t CUDAAPI tcudnnCreateAlgorithmPerformance(cudnnAlgorithmPerformance_t* algoPerf, int numberToCreate);
typedef cudnnStatus_t CUDAAPI tcudnnSetAlgorithmPerformance(cudnnAlgorithmPerformance_t algoPerf, cudnnAlgorithmDescriptor_t algoDesc, cudnnStatus_t status, float time, size_t memory);
typedef cudnnStatus_t CUDAAPI tcudnnGetAlgorithmPerformance(const cudnnAlgorithmPerformance_t algoPerf, cudnnAlgorithmDescriptor_t* algoDesc, cudnnStatus_t* status, float* time, size_t* memory);
typedef cudnnStatus_t CUDAAPI tcudnnDestroyAlgorithmPerformance(cudnnAlgorithmPerformance_t* algoPerf, int numberToDestroy);
typedef cudnnStatus_t CUDAAPI tcudnnGetAlgorithmSpaceSize(cudnnHandle_t handle, cudnnAlgorithmDescriptor_t algoDesc, size_t* algoSpaceSizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnSaveAlgorithm(cudnnHandle_t handle, cudnnAlgorithmDescriptor_t algoDesc, void* algoSpace, size_t algoSpaceSizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnRestoreAlgorithm(cudnnHandle_t handle, void* algoSpace, size_t algoSpaceSizeInBytes, cudnnAlgorithmDescriptor_t algoDesc);
typedef cudnnStatus_t CUDAAPI tcudnnSetCallback(unsigned mask, void* udata, cudnnCallback_t fptr);
typedef cudnnStatus_t CUDAAPI tcudnnGetCallback(unsigned* mask, void** udata, cudnnCallback_t* fptr);
typedef cudnnStatus_t CUDAAPI tcudnnOpsInferVersionCheck(void);
typedef cudnnStatus_t CUDAAPI tcudnnSoftmaxBackward(cudnnHandle_t handle, cudnnSoftmaxAlgorithm_t algo, cudnnSoftmaxMode_t mode, const void* alpha, const cudnnTensorDescriptor_t yDesc, const void* y, const cudnnTensorDescriptor_t dyDesc, const void* dy, const void* beta, const cudnnTensorDescriptor_t dxDesc, void* dx);
typedef cudnnStatus_t CUDAAPI tcudnnPoolingBackward(cudnnHandle_t handle, const cudnnPoolingDescriptor_t poolingDesc, const void* alpha, const cudnnTensorDescriptor_t yDesc, const void* y, const cudnnTensorDescriptor_t dyDesc, const void* dy, const cudnnTensorDescriptor_t xDesc, const void* x, const void* beta, const cudnnTensorDescriptor_t dxDesc, void* dx);
typedef cudnnStatus_t CUDAAPI tcudnnActivationBackward(cudnnHandle_t handle, cudnnActivationDescriptor_t activationDesc, const void* alpha, const cudnnTensorDescriptor_t yDesc, const void* y, const cudnnTensorDescriptor_t dyDesc, const void* dy, const cudnnTensorDescriptor_t xDesc, const void* x, const void* beta, const cudnnTensorDescriptor_t dxDesc, void* dx);
typedef cudnnStatus_t CUDAAPI tcudnnLRNCrossChannelBackward(cudnnHandle_t handle, cudnnLRNDescriptor_t normDesc, cudnnLRNMode_t lrnMode, const void* alpha, const cudnnTensorDescriptor_t yDesc, const void* y, const cudnnTensorDescriptor_t dyDesc, const void* dy, const cudnnTensorDescriptor_t xDesc, const void* x, const void* beta, const cudnnTensorDescriptor_t dxDesc, void* dx);
typedef cudnnStatus_t CUDAAPI tcudnnDivisiveNormalizationBackward(cudnnHandle_t handle, cudnnLRNDescriptor_t normDesc, cudnnDivNormMode_t mode, const void* alpha, const cudnnTensorDescriptor_t xDesc, const void* x, const void* means, const void* dy, void* temp, void* temp2, const void* beta, const cudnnTensorDescriptor_t dXdMeansDesc, void* dx, void* dMeans);
typedef cudnnStatus_t CUDAAPI tcudnnGetBatchNormalizationForwardTrainingExWorkspaceSize(cudnnHandle_t handle, cudnnBatchNormMode_t mode, cudnnBatchNormOps_t bnOps, const cudnnTensorDescriptor_t xDesc, const cudnnTensorDescriptor_t zDesc, const cudnnTensorDescriptor_t yDesc, const cudnnTensorDescriptor_t bnScaleBiasMeanVarDesc, const cudnnActivationDescriptor_t activationDesc, size_t* sizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnGetBatchNormalizationBackwardExWorkspaceSize(cudnnHandle_t handle, cudnnBatchNormMode_t mode, cudnnBatchNormOps_t bnOps, const cudnnTensorDescriptor_t xDesc, const cudnnTensorDescriptor_t yDesc, const cudnnTensorDescriptor_t dyDesc, const cudnnTensorDescriptor_t dzDesc, const cudnnTensorDescriptor_t dxDesc, const cudnnTensorDescriptor_t dBnScaleBiasDesc, const cudnnActivationDescriptor_t activationDesc, size_t* sizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnGetBatchNormalizationTrainingExReserveSpaceSize(cudnnHandle_t handle, cudnnBatchNormMode_t mode, cudnnBatchNormOps_t bnOps, const cudnnActivationDescriptor_t activationDesc, const cudnnTensorDescriptor_t xDesc, size_t* sizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnBatchNormalizationForwardTraining(cudnnHandle_t handle, cudnnBatchNormMode_t mode, const void* alpha, const void* beta, const cudnnTensorDescriptor_t xDesc, const void* x, const cudnnTensorDescriptor_t yDesc, void* y, const cudnnTensorDescriptor_t bnScaleBiasMeanVarDesc, const void* bnScale, const void* bnBias, double exponentialAverageFactor, void* resultRunningMean, void* resultRunningVariance, double epsilon, void* resultSaveMean, void* resultSaveInvVariance);
typedef cudnnStatus_t CUDAAPI tcudnnBatchNormalizationForwardTrainingEx(cudnnHandle_t handle, cudnnBatchNormMode_t mode, cudnnBatchNormOps_t bnOps, const void* alpha, const void* beta, const cudnnTensorDescriptor_t xDesc, const void* xData, const cudnnTensorDescriptor_t zDesc, const void* zData, const cudnnTensorDescriptor_t yDesc, void* yData, const cudnnTensorDescriptor_t bnScaleBiasMeanVarDesc, const void* bnScale, const void* bnBias, double exponentialAverageFactor, void* resultRunningMean, void* resultRunningVariance, double epsilon, void* resultSaveMean, void* resultSaveInvVariance, cudnnActivationDescriptor_t activationDesc, void* workspace, size_t workSpaceSizeInBytes, void* reserveSpace, size_t reserveSpaceSizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnBatchNormalizationBackward(cudnnHandle_t handle, cudnnBatchNormMode_t mode, const void* alphaDataDiff, const void* betaDataDiff, const void* alphaParamDiff, const void* betaParamDiff, const cudnnTensorDescriptor_t xDesc, const void* x, const cudnnTensorDescriptor_t dyDesc, const void* dy, const cudnnTensorDescriptor_t dxDesc, void* dx, const cudnnTensorDescriptor_t dBnScaleBiasDesc, const void* bnScale, void* dBnScaleResult, void* dBnBiasResult, double epsilon, const void* savedMean, const void* savedInvVariance);
typedef cudnnStatus_t CUDAAPI tcudnnBatchNormalizationBackwardEx(cudnnHandle_t handle, cudnnBatchNormMode_t mode, cudnnBatchNormOps_t bnOps, const void* alphaDataDiff, const void* betaDataDiff, const void* alphaParamDiff, const void* betaParamDiff, const cudnnTensorDescriptor_t xDesc, const void* xData, const cudnnTensorDescriptor_t yDesc, const void* yData, const cudnnTensorDescriptor_t dyDesc, const void* dyData, const cudnnTensorDescriptor_t dzDesc, void* dzData, const cudnnTensorDescriptor_t dxDesc, void* dxData, const cudnnTensorDescriptor_t dBnScaleBiasDesc, const void* bnScaleData, const void* bnBiasData, void* dBnScaleData, void* dBnBiasData, double epsilon, const void* savedMean, const void* savedInvVariance, cudnnActivationDescriptor_t activationDesc, void* workSpace, size_t workSpaceSizeInBytes, void* reserveSpace, size_t reserveSpaceSizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnGetNormalizationForwardTrainingWorkspaceSize(cudnnHandle_t handle, cudnnNormMode_t mode, cudnnNormOps_t normOps, cudnnNormAlgo_t algo, const cudnnTensorDescriptor_t xDesc, const cudnnTensorDescriptor_t zDesc, const cudnnTensorDescriptor_t yDesc, const cudnnTensorDescriptor_t normScaleBiasDesc, const cudnnActivationDescriptor_t activationDesc, const cudnnTensorDescriptor_t normMeanVarDesc, size_t* sizeInBytes, int groupCnt);
typedef cudnnStatus_t CUDAAPI tcudnnGetNormalizationBackwardWorkspaceSize(cudnnHandle_t handle, cudnnNormMode_t mode, cudnnNormOps_t normOps, cudnnNormAlgo_t algo, const cudnnTensorDescriptor_t xDesc, const cudnnTensorDescriptor_t yDesc, const cudnnTensorDescriptor_t dyDesc, const cudnnTensorDescriptor_t dzDesc, const cudnnTensorDescriptor_t dxDesc, const cudnnTensorDescriptor_t dNormScaleBiasDesc, const cudnnActivationDescriptor_t activationDesc, const cudnnTensorDescriptor_t normMeanVarDesc, size_t* sizeInBytes, int groupCnt);
typedef cudnnStatus_t CUDAAPI tcudnnGetNormalizationTrainingReserveSpaceSize(cudnnHandle_t handle, cudnnNormMode_t mode, cudnnNormOps_t normOps, cudnnNormAlgo_t algo, const cudnnActivationDescriptor_t activationDesc, const cudnnTensorDescriptor_t xDesc, size_t* sizeInBytes, int groupCnt);
typedef cudnnStatus_t CUDAAPI tcudnnNormalizationForwardTraining(cudnnHandle_t handle, cudnnNormMode_t mode, cudnnNormOps_t normOps, cudnnNormAlgo_t algo, const void* alpha, const void* beta, const cudnnTensorDescriptor_t xDesc, const void* xData, const cudnnTensorDescriptor_t normScaleBiasDesc, const void* normScale, const void* normBias, double exponentialAverageFactor, const cudnnTensorDescriptor_t normMeanVarDesc, void* resultRunningMean, void* resultRunningVariance, double epsilon, void* resultSaveMean, void* resultSaveInvVariance, cudnnActivationDescriptor_t activationDesc, const cudnnTensorDescriptor_t zDesc, const void* zData, const cudnnTensorDescriptor_t yDesc, void* yData, void* workspace, size_t workSpaceSizeInBytes, void* reserveSpace, size_t reserveSpaceSizeInBytes, int groupCnt);
typedef cudnnStatus_t CUDAAPI tcudnnNormalizationBackward(cudnnHandle_t handle, cudnnNormMode_t mode, cudnnNormOps_t normOps, cudnnNormAlgo_t algo, const void* alphaDataDiff, const void* betaDataDiff, const void* alphaParamDiff, const void* betaParamDiff, const cudnnTensorDescriptor_t xDesc, const void* xData, const cudnnTensorDescriptor_t yDesc, const void* yData, const cudnnTensorDescriptor_t dyDesc, const void* dyData, const cudnnTensorDescriptor_t dzDesc, void* dzData, const cudnnTensorDescriptor_t dxDesc, void* dxData, const cudnnTensorDescriptor_t dNormScaleBiasDesc, const void* normScaleData, const void* normBiasData, void* dNormScaleData, void* dNormBiasData, double epsilon, const cudnnTensorDescriptor_t normMeanVarDesc, const void* savedMean, const void* savedInvVariance, cudnnActivationDescriptor_t activationDesc, void* workSpace, size_t workSpaceSizeInBytes, void* reserveSpace, size_t reserveSpaceSizeInBytes, int groupCnt);
typedef cudnnStatus_t CUDAAPI tcudnnSpatialTfGridGeneratorBackward(cudnnHandle_t handle, const cudnnSpatialTransformerDescriptor_t stDesc, const void* dgrid, void* dtheta);
typedef cudnnStatus_t CUDAAPI tcudnnSpatialTfSamplerBackward(cudnnHandle_t handle, cudnnSpatialTransformerDescriptor_t stDesc, const void* alpha, const cudnnTensorDescriptor_t xDesc, const void* x, const void* beta, const cudnnTensorDescriptor_t dxDesc, void* dx, const void* alphaDgrid, const cudnnTensorDescriptor_t dyDesc, const void* dy, const void* grid, const void* betaDgrid, void* dgrid);
typedef cudnnStatus_t CUDAAPI tcudnnDropoutBackward(cudnnHandle_t handle, const cudnnDropoutDescriptor_t dropoutDesc, const cudnnTensorDescriptor_t dydesc, const void* dy, const cudnnTensorDescriptor_t dxdesc, void* dx, void* reserveSpace, size_t reserveSpaceSizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnOpsTrainVersionCheck(void);
typedef cudnnStatus_t CUDAAPI tcudnnCreateRNNDescriptor(cudnnRNNDescriptor_t* rnnDesc);
typedef cudnnStatus_t CUDAAPI tcudnnDestroyRNNDescriptor(cudnnRNNDescriptor_t rnnDesc);
typedef cudnnStatus_t CUDAAPI tcudnnSetRNNDescriptor_v8(cudnnRNNDescriptor_t rnnDesc, cudnnRNNAlgo_t algo, cudnnRNNMode_t cellMode, cudnnRNNBiasMode_t biasMode, cudnnDirectionMode_t dirMode, cudnnRNNInputMode_t inputMode, cudnnDataType_t dataType, cudnnDataType_t mathPrec, cudnnMathType_t mathType, int32_t inputSize, int32_t hiddenSize, int32_t projSize, int32_t numLayers, cudnnDropoutDescriptor_t dropoutDesc, uint32_t auxFlags);
typedef cudnnStatus_t CUDAAPI tcudnnGetRNNDescriptor_v8(cudnnRNNDescriptor_t rnnDesc, cudnnRNNAlgo_t* algo, cudnnRNNMode_t* cellMode, cudnnRNNBiasMode_t* biasMode, cudnnDirectionMode_t* dirMode, cudnnRNNInputMode_t* inputMode, cudnnDataType_t* dataType, cudnnDataType_t* mathPrec, cudnnMathType_t* mathType, int32_t* inputSize, int32_t* hiddenSize, int32_t* projSize, int32_t* numLayers, cudnnDropoutDescriptor_t* dropoutDesc, uint32_t* auxFlags);
typedef cudnnStatus_t CUDAAPI tcudnnSetRNNDescriptor_v6(cudnnHandle_t handle, cudnnRNNDescriptor_t rnnDesc, const int hiddenSize, const int numLayers, cudnnDropoutDescriptor_t dropoutDesc, cudnnRNNInputMode_t inputMode, cudnnDirectionMode_t direction, cudnnRNNMode_t cellMode, cudnnRNNAlgo_t algo, cudnnDataType_t mathPrec);
typedef cudnnStatus_t CUDAAPI tcudnnGetRNNDescriptor_v6(cudnnHandle_t handle, cudnnRNNDescriptor_t rnnDesc, int* hiddenSize, int* numLayers, cudnnDropoutDescriptor_t* dropoutDesc, cudnnRNNInputMode_t* inputMode, cudnnDirectionMode_t* direction, cudnnRNNMode_t* cellMode, cudnnRNNAlgo_t* algo, cudnnDataType_t* mathPrec);
typedef cudnnStatus_t CUDAAPI tcudnnSetRNNMatrixMathType(cudnnRNNDescriptor_t rnnDesc, cudnnMathType_t mType);
typedef cudnnStatus_t CUDAAPI tcudnnGetRNNMatrixMathType(cudnnRNNDescriptor_t rnnDesc, cudnnMathType_t* mType);
typedef cudnnStatus_t CUDAAPI tcudnnSetRNNBiasMode(cudnnRNNDescriptor_t rnnDesc, cudnnRNNBiasMode_t biasMode);
typedef cudnnStatus_t CUDAAPI tcudnnGetRNNBiasMode(cudnnRNNDescriptor_t rnnDesc, cudnnRNNBiasMode_t* biasMode);
typedef cudnnStatus_t CUDAAPI tcudnnRNNSetClip_v8(cudnnRNNDescriptor_t rnnDesc, cudnnRNNClipMode_t clipMode, cudnnNanPropagation_t clipNanOpt, double lclip, double rclip);
typedef cudnnStatus_t CUDAAPI tcudnnRNNGetClip_v8(cudnnRNNDescriptor_t rnnDesc, cudnnRNNClipMode_t* clipMode, cudnnNanPropagation_t* clipNanOpt, double* lclip, double* rclip);
typedef cudnnStatus_t CUDAAPI tcudnnRNNSetClip(cudnnHandle_t handle, cudnnRNNDescriptor_t rnnDesc, cudnnRNNClipMode_t clipMode, cudnnNanPropagation_t clipNanOpt, double lclip, double rclip);
typedef cudnnStatus_t CUDAAPI tcudnnRNNGetClip(cudnnHandle_t handle, cudnnRNNDescriptor_t rnnDesc, cudnnRNNClipMode_t* clipMode, cudnnNanPropagation_t* clipNanOpt, double* lclip, double* rclip);
typedef cudnnStatus_t CUDAAPI tcudnnSetRNNProjectionLayers(cudnnHandle_t handle, cudnnRNNDescriptor_t rnnDesc, const int recProjSize, const int outProjSize);
typedef cudnnStatus_t CUDAAPI tcudnnGetRNNProjectionLayers(cudnnHandle_t handle, const cudnnRNNDescriptor_t rnnDesc, int* recProjSize, int* outProjSize);
typedef cudnnStatus_t CUDAAPI tcudnnCreatePersistentRNNPlan(cudnnRNNDescriptor_t rnnDesc, const int minibatch, const cudnnDataType_t dataType, cudnnPersistentRNNPlan_t* plan);
typedef cudnnStatus_t CUDAAPI tcudnnDestroyPersistentRNNPlan(cudnnPersistentRNNPlan_t plan);
typedef cudnnStatus_t CUDAAPI tcudnnSetPersistentRNNPlan(cudnnRNNDescriptor_t rnnDesc, cudnnPersistentRNNPlan_t plan);
typedef cudnnStatus_t CUDAAPI tcudnnBuildRNNDynamic(cudnnHandle_t handle, cudnnRNNDescriptor_t rnnDesc, int miniBatch);
typedef cudnnStatus_t CUDAAPI tcudnnGetRNNWorkspaceSize(cudnnHandle_t handle, const cudnnRNNDescriptor_t rnnDesc, const int seqLength, const cudnnTensorDescriptor_t* xDesc, size_t* sizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnGetRNNTrainingReserveSize(cudnnHandle_t handle, const cudnnRNNDescriptor_t rnnDesc, const int seqLength, const cudnnTensorDescriptor_t* xDesc, size_t* sizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnGetRNNTempSpaceSizes(cudnnHandle_t handle, cudnnRNNDescriptor_t rnnDesc, cudnnForwardMode_t fMode, cudnnRNNDataDescriptor_t xDesc, size_t* workSpaceSize, size_t* reserveSpaceSize);
typedef cudnnStatus_t CUDAAPI tcudnnGetRNNParamsSize(cudnnHandle_t handle, const cudnnRNNDescriptor_t rnnDesc, const cudnnTensorDescriptor_t xDesc, size_t* sizeInBytes, cudnnDataType_t dataType);
typedef cudnnStatus_t CUDAAPI tcudnnGetRNNWeightSpaceSize(cudnnHandle_t handle, cudnnRNNDescriptor_t rnnDesc, size_t* weightSpaceSize);
typedef cudnnStatus_t CUDAAPI tcudnnGetRNNLinLayerMatrixParams(cudnnHandle_t handle, const cudnnRNNDescriptor_t rnnDesc, const int pseudoLayer, const cudnnTensorDescriptor_t xDesc, const cudnnFilterDescriptor_t wDesc, const void* w, const int linLayerID, cudnnFilterDescriptor_t linLayerMatDesc, void** linLayerMat);
typedef cudnnStatus_t CUDAAPI tcudnnGetRNNLinLayerBiasParams(cudnnHandle_t handle, const cudnnRNNDescriptor_t rnnDesc, const int pseudoLayer, const cudnnTensorDescriptor_t xDesc, const cudnnFilterDescriptor_t wDesc, const void* w, const int linLayerID, cudnnFilterDescriptor_t linLayerBiasDesc, void** linLayerBias);
typedef cudnnStatus_t CUDAAPI tcudnnGetRNNWeightParams(cudnnHandle_t handle, cudnnRNNDescriptor_t rnnDesc, int32_t pseudoLayer, size_t weightSpaceSize, const void* weightSpace, int32_t linLayerID, cudnnTensorDescriptor_t mDesc, void** mAddr, cudnnTensorDescriptor_t bDesc, void** bAddr);
typedef cudnnStatus_t CUDAAPI tcudnnRNNForwardInference(cudnnHandle_t handle, const cudnnRNNDescriptor_t rnnDesc, const int seqLength, const cudnnTensorDescriptor_t* xDesc, const void* x, const cudnnTensorDescriptor_t hxDesc, const void* hx, const cudnnTensorDescriptor_t cxDesc, const void* cx, const cudnnFilterDescriptor_t wDesc, const void* w, const cudnnTensorDescriptor_t* yDesc, void* y, const cudnnTensorDescriptor_t hyDesc, void* hy, const cudnnTensorDescriptor_t cyDesc, void* cy, void* workSpace, size_t workSpaceSizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnSetRNNPaddingMode(cudnnRNNDescriptor_t rnnDesc, unsigned paddingMode);
typedef cudnnStatus_t CUDAAPI tcudnnGetRNNPaddingMode(cudnnRNNDescriptor_t rnnDesc, unsigned* paddingMode);
typedef cudnnStatus_t CUDAAPI tcudnnCreateRNNDataDescriptor(cudnnRNNDataDescriptor_t* rnnDataDesc);
typedef cudnnStatus_t CUDAAPI tcudnnDestroyRNNDataDescriptor(cudnnRNNDataDescriptor_t rnnDataDesc);
typedef cudnnStatus_t CUDAAPI tcudnnSetRNNDataDescriptor(cudnnRNNDataDescriptor_t rnnDataDesc, cudnnDataType_t dataType, cudnnRNNDataLayout_t layout, int maxSeqLength, int batchSize, int vectorSize, const int seqLengthArray[], void* paddingFill);
typedef cudnnStatus_t CUDAAPI tcudnnGetRNNDataDescriptor(cudnnRNNDataDescriptor_t rnnDataDesc, cudnnDataType_t* dataType, cudnnRNNDataLayout_t* layout, int* maxSeqLength, int* batchSize, int* vectorSize, int arrayLengthRequested, int seqLengthArray[], void* paddingFill);
typedef cudnnStatus_t CUDAAPI tcudnnRNNForwardInferenceEx(cudnnHandle_t handle, const cudnnRNNDescriptor_t rnnDesc, const cudnnRNNDataDescriptor_t xDesc, const void* x, const cudnnTensorDescriptor_t hxDesc, const void* hx, const cudnnTensorDescriptor_t cxDesc, const void* cx, const cudnnFilterDescriptor_t wDesc, const void* w, const cudnnRNNDataDescriptor_t yDesc, void* y, const cudnnTensorDescriptor_t hyDesc, void* hy, const cudnnTensorDescriptor_t cyDesc, void* cy, const cudnnRNNDataDescriptor_t kDesc, const void* keys, const cudnnRNNDataDescriptor_t cDesc, void* cAttn, const cudnnRNNDataDescriptor_t iDesc, void* iAttn, const cudnnRNNDataDescriptor_t qDesc, void* queries, void* workSpace, size_t workSpaceSizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnRNNForward(cudnnHandle_t handle, cudnnRNNDescriptor_t rnnDesc, cudnnForwardMode_t fwdMode, const int32_t devSeqLengths[], cudnnRNNDataDescriptor_t xDesc, const void* x, cudnnRNNDataDescriptor_t yDesc, void* y, cudnnTensorDescriptor_t hDesc, const void* hx, void* hy, cudnnTensorDescriptor_t cDesc, const void* cx, void* cy, size_t weightSpaceSize, const void* weightSpace, size_t workSpaceSize, void* workSpace, size_t reserveSpaceSize, void* reserveSpace);
typedef cudnnStatus_t CUDAAPI tcudnnSetRNNAlgorithmDescriptor(cudnnHandle_t handle, cudnnRNNDescriptor_t rnnDesc, cudnnAlgorithmDescriptor_t algoDesc);
typedef cudnnStatus_t CUDAAPI tcudnnGetRNNForwardInferenceAlgorithmMaxCount(cudnnHandle_t handle, const cudnnRNNDescriptor_t rnnDesc, int* count);
typedef cudnnStatus_t CUDAAPI tcudnnFindRNNForwardInferenceAlgorithmEx(cudnnHandle_t handle, const cudnnRNNDescriptor_t rnnDesc, const int seqLength, const cudnnTensorDescriptor_t* xDesc, const void* x, const cudnnTensorDescriptor_t hxDesc, const void* hx, const cudnnTensorDescriptor_t cxDesc, const void* cx, const cudnnFilterDescriptor_t wDesc, const void* w, const cudnnTensorDescriptor_t* yDesc, void* y, const cudnnTensorDescriptor_t hyDesc, void* hy, const cudnnTensorDescriptor_t cyDesc, void* cy, const float findIntensity, const int requestedAlgoCount, int* returnedAlgoCount, cudnnAlgorithmPerformance_t* perfResults, void* workspace, size_t workSpaceSizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnCreateSeqDataDescriptor(cudnnSeqDataDescriptor_t* seqDataDesc);
typedef cudnnStatus_t CUDAAPI tcudnnDestroySeqDataDescriptor(cudnnSeqDataDescriptor_t seqDataDesc);
typedef cudnnStatus_t CUDAAPI tcudnnSetSeqDataDescriptor(cudnnSeqDataDescriptor_t seqDataDesc, cudnnDataType_t dataType, int nbDims, const int dimA[], const cudnnSeqDataAxis_t axes[], size_t seqLengthArraySize, const int seqLengthArray[], void* paddingFill);
typedef cudnnStatus_t CUDAAPI tcudnnGetSeqDataDescriptor(const cudnnSeqDataDescriptor_t seqDataDesc, cudnnDataType_t* dataType, int* nbDims, int nbDimsRequested, int dimA[], cudnnSeqDataAxis_t axes[], size_t* seqLengthArraySize, size_t seqLengthSizeRequested, int seqLengthArray[], void* paddingFill);
typedef cudnnStatus_t CUDAAPI tcudnnCreateAttnDescriptor(cudnnAttnDescriptor_t* attnDesc);
typedef cudnnStatus_t CUDAAPI tcudnnDestroyAttnDescriptor(cudnnAttnDescriptor_t attnDesc);
typedef cudnnStatus_t CUDAAPI tcudnnSetAttnDescriptor(cudnnAttnDescriptor_t attnDesc, unsigned attnMode, int nHeads, double smScaler, cudnnDataType_t dataType, cudnnDataType_t computePrec, cudnnMathType_t mathType, cudnnDropoutDescriptor_t attnDropoutDesc, cudnnDropoutDescriptor_t postDropoutDesc, int qSize, int kSize, int vSize, int qProjSize, int kProjSize, int vProjSize, int oProjSize, int qoMaxSeqLength, int kvMaxSeqLength, int maxBatchSize, int maxBeamSize);
typedef cudnnStatus_t CUDAAPI tcudnnGetAttnDescriptor(cudnnAttnDescriptor_t attnDesc, unsigned* attnMode, int* nHeads, double* smScaler, cudnnDataType_t* dataType, cudnnDataType_t* computePrec, cudnnMathType_t* mathType, cudnnDropoutDescriptor_t* attnDropoutDesc, cudnnDropoutDescriptor_t* postDropoutDesc, int* qSize, int* kSize, int* vSize, int* qProjSize, int* kProjSize, int* vProjSize, int* oProjSize, int* qoMaxSeqLength, int* kvMaxSeqLength, int* maxBatchSize, int* maxBeamSize);
typedef cudnnStatus_t CUDAAPI tcudnnGetMultiHeadAttnBuffers(cudnnHandle_t handle, const cudnnAttnDescriptor_t attnDesc, size_t* weightSizeInBytes, size_t* workSpaceSizeInBytes, size_t* reserveSpaceSizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnGetMultiHeadAttnWeights(cudnnHandle_t handle, const cudnnAttnDescriptor_t attnDesc, cudnnMultiHeadAttnWeightKind_t wKind, size_t weightSizeInBytes, const void* weights, cudnnTensorDescriptor_t wDesc, void** wAddr);
typedef cudnnStatus_t CUDAAPI tcudnnMultiHeadAttnForward(cudnnHandle_t handle, const cudnnAttnDescriptor_t attnDesc, int currIdx, const int loWinIdx[], const int hiWinIdx[], const int devSeqLengthsQO[], const int devSeqLengthsKV[], const cudnnSeqDataDescriptor_t qDesc, const void* queries, const void* residuals, const cudnnSeqDataDescriptor_t kDesc, const void* keys, const cudnnSeqDataDescriptor_t vDesc, const void* values, const cudnnSeqDataDescriptor_t oDesc, void* out, size_t weightSizeInBytes, const void* weights, size_t workSpaceSizeInBytes, void* workSpace, size_t reserveSpaceSizeInBytes, void* reserveSpace);
typedef cudnnStatus_t CUDAAPI tcudnnAdvInferVersionCheck(void);
typedef cudnnStatus_t CUDAAPI tcudnnRNNForwardTraining(cudnnHandle_t handle, const cudnnRNNDescriptor_t rnnDesc, const int seqLength, const cudnnTensorDescriptor_t* xDesc, const void* x, const cudnnTensorDescriptor_t hxDesc, const void* hx, const cudnnTensorDescriptor_t cxDesc, const void* cx, const cudnnFilterDescriptor_t wDesc, const void* w, const cudnnTensorDescriptor_t* yDesc, void* y, const cudnnTensorDescriptor_t hyDesc, void* hy, const cudnnTensorDescriptor_t cyDesc, void* cy, void* workSpace, size_t workSpaceSizeInBytes, void* reserveSpace, size_t reserveSpaceSizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnRNNBackwardData(cudnnHandle_t handle, const cudnnRNNDescriptor_t rnnDesc, const int seqLength, const cudnnTensorDescriptor_t* yDesc, const void* y, const cudnnTensorDescriptor_t* dyDesc, const void* dy, const cudnnTensorDescriptor_t dhyDesc, const void* dhy, const cudnnTensorDescriptor_t dcyDesc, const void* dcy, const cudnnFilterDescriptor_t wDesc, const void* w, const cudnnTensorDescriptor_t hxDesc, const void* hx, const cudnnTensorDescriptor_t cxDesc, const void* cx, const cudnnTensorDescriptor_t* dxDesc, void* dx, const cudnnTensorDescriptor_t dhxDesc, void* dhx, const cudnnTensorDescriptor_t dcxDesc, void* dcx, void* workSpace, size_t workSpaceSizeInBytes, void* reserveSpace, size_t reserveSpaceSizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnRNNBackwardData_v8(cudnnHandle_t handle, cudnnRNNDescriptor_t rnnDesc, const int32_t devSeqLengths[], cudnnRNNDataDescriptor_t yDesc, const void* y, const void* dy, cudnnRNNDataDescriptor_t xDesc, void* dx, cudnnTensorDescriptor_t hDesc, const void* hx, const void* dhy, void* dhx, cudnnTensorDescriptor_t cDesc, const void* cx, const void* dcy, void* dcx, size_t weightSpaceSize, const void* weightSpace, size_t workSpaceSize, void* workSpace, size_t reserveSpaceSize, void* reserveSpace);
typedef cudnnStatus_t CUDAAPI tcudnnRNNBackwardWeights(cudnnHandle_t handle, const cudnnRNNDescriptor_t rnnDesc, const int seqLength, const cudnnTensorDescriptor_t* xDesc, const void* x, const cudnnTensorDescriptor_t hxDesc, const void* hx, const cudnnTensorDescriptor_t* yDesc, const void* y, const void* workSpace, size_t workSpaceSizeInBytes, const cudnnFilterDescriptor_t dwDesc, void* dw, const void* reserveSpace, size_t reserveSpaceSizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnRNNBackwardWeights_v8(cudnnHandle_t handle, cudnnRNNDescriptor_t rnnDesc, cudnnWgradMode_t addGrad, const int32_t devSeqLengths[], cudnnRNNDataDescriptor_t xDesc, const void* x, cudnnTensorDescriptor_t hDesc, const void* hx, cudnnRNNDataDescriptor_t yDesc, const void* y, size_t weightSpaceSize, void* dweightSpace, size_t workSpaceSize, void* workSpace, size_t reserveSpaceSize, void* reserveSpace);
typedef cudnnStatus_t CUDAAPI tcudnnRNNForwardTrainingEx(cudnnHandle_t handle, const cudnnRNNDescriptor_t rnnDesc, const cudnnRNNDataDescriptor_t xDesc, const void* x, const cudnnTensorDescriptor_t hxDesc, const void* hx, const cudnnTensorDescriptor_t cxDesc, const void* cx, const cudnnFilterDescriptor_t wDesc, const void* w, const cudnnRNNDataDescriptor_t yDesc, void* y, const cudnnTensorDescriptor_t hyDesc, void* hy, const cudnnTensorDescriptor_t cyDesc, void* cy, const cudnnRNNDataDescriptor_t kDesc, const void* keys, const cudnnRNNDataDescriptor_t cDesc, void* cAttn, const cudnnRNNDataDescriptor_t iDesc, void* iAttn, const cudnnRNNDataDescriptor_t qDesc, void* queries, void* workSpace, size_t workSpaceSizeInBytes, void* reserveSpace, size_t reserveSpaceSizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnRNNBackwardDataEx(cudnnHandle_t handle, const cudnnRNNDescriptor_t rnnDesc, const cudnnRNNDataDescriptor_t yDesc, const void* y, const cudnnRNNDataDescriptor_t dyDesc, const void* dy, const cudnnRNNDataDescriptor_t dcDesc, const void* dcAttn, const cudnnTensorDescriptor_t dhyDesc, const void* dhy, const cudnnTensorDescriptor_t dcyDesc, const void* dcy, const cudnnFilterDescriptor_t wDesc, const void* w, const cudnnTensorDescriptor_t hxDesc, const void* hx, const cudnnTensorDescriptor_t cxDesc, const void* cx, const cudnnRNNDataDescriptor_t dxDesc, void* dx, const cudnnTensorDescriptor_t dhxDesc, void* dhx, const cudnnTensorDescriptor_t dcxDesc, void* dcx, const cudnnRNNDataDescriptor_t dkDesc, void* dkeys, void* workSpace, size_t workSpaceSizeInBytes, void* reserveSpace, size_t reserveSpaceSizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnRNNBackwardWeightsEx(cudnnHandle_t handle, const cudnnRNNDescriptor_t rnnDesc, const cudnnRNNDataDescriptor_t xDesc, const void* x, const cudnnTensorDescriptor_t hxDesc, const void* hx, const cudnnRNNDataDescriptor_t yDesc, const void* y, void* workSpace, size_t workSpaceSizeInBytes, const cudnnFilterDescriptor_t dwDesc, void* dw, void* reserveSpace, size_t reserveSpaceSizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnGetRNNForwardTrainingAlgorithmMaxCount(cudnnHandle_t handle, const cudnnRNNDescriptor_t rnnDesc, int* count);
typedef cudnnStatus_t CUDAAPI tcudnnFindRNNForwardTrainingAlgorithmEx(cudnnHandle_t handle, const cudnnRNNDescriptor_t rnnDesc, const int seqLength, const cudnnTensorDescriptor_t* xDesc, const void* x, const cudnnTensorDescriptor_t hxDesc, const void* hx, const cudnnTensorDescriptor_t cxDesc, const void* cx, const cudnnFilterDescriptor_t wDesc, const void* w, const cudnnTensorDescriptor_t* yDesc, void* y, const cudnnTensorDescriptor_t hyDesc, void* hy, const cudnnTensorDescriptor_t cyDesc, void* cy, const float findIntensity, const int requestedAlgoCount, int* returnedAlgoCount, cudnnAlgorithmPerformance_t* perfResults, void* workspace, size_t workSpaceSizeInBytes, void* reserveSpace, size_t reserveSpaceSizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnGetRNNBackwardDataAlgorithmMaxCount(cudnnHandle_t handle, const cudnnRNNDescriptor_t rnnDesc, int* count);
typedef cudnnStatus_t CUDAAPI tcudnnFindRNNBackwardDataAlgorithmEx(cudnnHandle_t handle, const cudnnRNNDescriptor_t rnnDesc, const int seqLength, const cudnnTensorDescriptor_t* yDesc, const void* y, const cudnnTensorDescriptor_t* dyDesc, const void* dy, const cudnnTensorDescriptor_t dhyDesc, const void* dhy, const cudnnTensorDescriptor_t dcyDesc, const void* dcy, const cudnnFilterDescriptor_t wDesc, const void* w, const cudnnTensorDescriptor_t hxDesc, const void* hx, const cudnnTensorDescriptor_t cxDesc, const void* cx, const cudnnTensorDescriptor_t* dxDesc, void* dx, const cudnnTensorDescriptor_t dhxDesc, void* dhx, const cudnnTensorDescriptor_t dcxDesc, void* dcx, const float findIntensity, const int requestedAlgoCount, int* returnedAlgoCount, cudnnAlgorithmPerformance_t* perfResults, void* workspace, size_t workSpaceSizeInBytes, void* reserveSpace, size_t reserveSpaceSizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnGetRNNBackwardWeightsAlgorithmMaxCount(cudnnHandle_t handle, const cudnnRNNDescriptor_t rnnDesc, int* count);
typedef cudnnStatus_t CUDAAPI tcudnnFindRNNBackwardWeightsAlgorithmEx(cudnnHandle_t handle, const cudnnRNNDescriptor_t rnnDesc, const int seqLength, const cudnnTensorDescriptor_t* xDesc, const void* x, const cudnnTensorDescriptor_t hxDesc, const void* hx, const cudnnTensorDescriptor_t* yDesc, const void* y, const float findIntensity, const int requestedAlgoCount, int* returnedAlgoCount, cudnnAlgorithmPerformance_t* perfResults, const void* workspace, size_t workSpaceSizeInBytes, const cudnnFilterDescriptor_t dwDesc, void* dw, const void* reserveSpace, size_t reserveSpaceSizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnMultiHeadAttnBackwardData(cudnnHandle_t handle, const cudnnAttnDescriptor_t attnDesc, const int loWinIdx[], const int hiWinIdx[], const int devSeqLengthsDQDO[], const int devSeqLengthsDKDV[], const cudnnSeqDataDescriptor_t doDesc, const void* dout, const cudnnSeqDataDescriptor_t dqDesc, void* dqueries, const void* queries, const cudnnSeqDataDescriptor_t dkDesc, void* dkeys, const void* keys, const cudnnSeqDataDescriptor_t dvDesc, void* dvalues, const void* values, size_t weightSizeInBytes, const void* weights, size_t workSpaceSizeInBytes, void* workSpace, size_t reserveSpaceSizeInBytes, void* reserveSpace);
typedef cudnnStatus_t CUDAAPI tcudnnMultiHeadAttnBackwardWeights(cudnnHandle_t handle, const cudnnAttnDescriptor_t attnDesc, cudnnWgradMode_t addGrad, const cudnnSeqDataDescriptor_t qDesc, const void* queries, const cudnnSeqDataDescriptor_t kDesc, const void* keys, const cudnnSeqDataDescriptor_t vDesc, const void* values, const cudnnSeqDataDescriptor_t doDesc, const void* dout, size_t weightSizeInBytes, const void* weights, void* dweights, size_t workSpaceSizeInBytes, void* workSpace, size_t reserveSpaceSizeInBytes, void* reserveSpace);
typedef cudnnStatus_t CUDAAPI tcudnnCreateCTCLossDescriptor(cudnnCTCLossDescriptor_t* ctcLossDesc);
typedef cudnnStatus_t CUDAAPI tcudnnSetCTCLossDescriptor(cudnnCTCLossDescriptor_t ctcLossDesc, cudnnDataType_t compType);
typedef cudnnStatus_t CUDAAPI tcudnnSetCTCLossDescriptorEx(cudnnCTCLossDescriptor_t ctcLossDesc, cudnnDataType_t compType, cudnnLossNormalizationMode_t normMode, cudnnNanPropagation_t gradMode);
typedef cudnnStatus_t CUDAAPI tcudnnSetCTCLossDescriptor_v8(cudnnCTCLossDescriptor_t ctcLossDesc, cudnnDataType_t compType, cudnnLossNormalizationMode_t normMode, cudnnNanPropagation_t gradMode, int maxLabelLength);
typedef cudnnStatus_t CUDAAPI tcudnnGetCTCLossDescriptor(cudnnCTCLossDescriptor_t ctcLossDesc, cudnnDataType_t* compType);
typedef cudnnStatus_t CUDAAPI tcudnnGetCTCLossDescriptorEx(cudnnCTCLossDescriptor_t ctcLossDesc, cudnnDataType_t* compType, cudnnLossNormalizationMode_t* normMode, cudnnNanPropagation_t* gradMode);
typedef cudnnStatus_t CUDAAPI tcudnnGetCTCLossDescriptor_v8(cudnnCTCLossDescriptor_t ctcLossDesc, cudnnDataType_t* compType, cudnnLossNormalizationMode_t* normMode, cudnnNanPropagation_t* gradMode, int* maxLabelLength);
typedef cudnnStatus_t CUDAAPI tcudnnDestroyCTCLossDescriptor(cudnnCTCLossDescriptor_t ctcLossDesc);
typedef cudnnStatus_t CUDAAPI tcudnnCTCLoss(cudnnHandle_t handle, const cudnnTensorDescriptor_t probsDesc, const void* probs, const int hostLabels[], const int hostLabelLengths[], const int hostInputLengths[], void* costs, const cudnnTensorDescriptor_t gradientsDesc, void* gradients, cudnnCTCLossAlgo_t algo, cudnnCTCLossDescriptor_t ctcLossDesc, void* workspace, size_t workSpaceSizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnCTCLoss_v8(cudnnHandle_t handle, cudnnCTCLossAlgo_t algo, cudnnCTCLossDescriptor_t ctcLossDesc, const cudnnTensorDescriptor_t probsDesc, const void* probs, const int labels[], const int labelLengths[], const int inputLengths[], void* costs, const cudnnTensorDescriptor_t gradientsDesc, void* gradients, size_t workSpaceSizeInBytes, void* workspace);
typedef cudnnStatus_t CUDAAPI tcudnnGetCTCLossWorkspaceSize(cudnnHandle_t handle, const cudnnTensorDescriptor_t probsDesc, const cudnnTensorDescriptor_t gradientsDesc, const int* labels, const int* labelLengths, const int* inputLengths, cudnnCTCLossAlgo_t algo, cudnnCTCLossDescriptor_t ctcLossDesc, size_t* sizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnGetCTCLossWorkspaceSize_v8(cudnnHandle_t handle, cudnnCTCLossAlgo_t algo, cudnnCTCLossDescriptor_t ctcLossDesc, const cudnnTensorDescriptor_t probsDesc, const cudnnTensorDescriptor_t gradientsDesc, size_t* sizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnAdvTrainVersionCheck(void);
typedef cudnnStatus_t CUDAAPI tcudnnCreateConvolutionDescriptor(cudnnConvolutionDescriptor_t* convDesc);
typedef cudnnStatus_t CUDAAPI tcudnnDestroyConvolutionDescriptor(cudnnConvolutionDescriptor_t convDesc);
typedef cudnnStatus_t CUDAAPI tcudnnSetConvolutionMathType(cudnnConvolutionDescriptor_t convDesc, cudnnMathType_t mathType);
typedef cudnnStatus_t CUDAAPI tcudnnGetConvolutionMathType(cudnnConvolutionDescriptor_t convDesc, cudnnMathType_t* mathType);
typedef cudnnStatus_t CUDAAPI tcudnnSetConvolutionGroupCount(cudnnConvolutionDescriptor_t convDesc, int groupCount);
typedef cudnnStatus_t CUDAAPI tcudnnGetConvolutionGroupCount(cudnnConvolutionDescriptor_t convDesc, int* groupCount);
typedef cudnnStatus_t CUDAAPI tcudnnSetConvolutionReorderType(cudnnConvolutionDescriptor_t convDesc, cudnnReorderType_t reorderType);
typedef cudnnStatus_t CUDAAPI tcudnnGetConvolutionReorderType(cudnnConvolutionDescriptor_t convDesc, cudnnReorderType_t* reorderType);
typedef cudnnStatus_t CUDAAPI tcudnnSetConvolution2dDescriptor(cudnnConvolutionDescriptor_t convDesc, int pad_h, int pad_w, int u, int v, int dilation_h, int dilation_w, cudnnConvolutionMode_t mode, cudnnDataType_t computeType);
typedef cudnnStatus_t CUDAAPI tcudnnGetConvolution2dDescriptor(const cudnnConvolutionDescriptor_t convDesc, int* pad_h, int* pad_w, int* u, int* v, int* dilation_h, int* dilation_w, cudnnConvolutionMode_t* mode, cudnnDataType_t* computeType);
typedef cudnnStatus_t CUDAAPI tcudnnSetConvolutionNdDescriptor(cudnnConvolutionDescriptor_t convDesc, int arrayLength, const int padA[], const int filterStrideA[], const int dilationA[], cudnnConvolutionMode_t mode, cudnnDataType_t computeType);
typedef cudnnStatus_t CUDAAPI tcudnnGetConvolutionNdDescriptor(const cudnnConvolutionDescriptor_t convDesc, int arrayLengthRequested, int* arrayLength, int padA[], int strideA[], int dilationA[], cudnnConvolutionMode_t* mode, cudnnDataType_t* computeType);
typedef cudnnStatus_t CUDAAPI tcudnnGetConvolution2dForwardOutputDim(const cudnnConvolutionDescriptor_t convDesc, const cudnnTensorDescriptor_t inputTensorDesc, const cudnnFilterDescriptor_t filterDesc, int* n, int* c, int* h, int* w);
typedef cudnnStatus_t CUDAAPI tcudnnGetConvolutionNdForwardOutputDim(const cudnnConvolutionDescriptor_t convDesc, const cudnnTensorDescriptor_t inputTensorDesc, const cudnnFilterDescriptor_t filterDesc, int nbDims, int tensorOuputDimA[]);
typedef cudnnStatus_t CUDAAPI tcudnnGetConvolutionForwardAlgorithmMaxCount(cudnnHandle_t handle, int* count);
typedef cudnnStatus_t CUDAAPI tcudnnGetConvolutionForwardAlgorithm_v7(cudnnHandle_t handle, const cudnnTensorDescriptor_t srcDesc, const cudnnFilterDescriptor_t filterDesc, const cudnnConvolutionDescriptor_t convDesc, const cudnnTensorDescriptor_t destDesc, const int requestedAlgoCount, int* returnedAlgoCount, cudnnConvolutionFwdAlgoPerf_t* perfResults);
typedef cudnnStatus_t CUDAAPI tcudnnFindConvolutionForwardAlgorithm(cudnnHandle_t handle, const cudnnTensorDescriptor_t xDesc, const cudnnFilterDescriptor_t wDesc, const cudnnConvolutionDescriptor_t convDesc, const cudnnTensorDescriptor_t yDesc, const int requestedAlgoCount, int* returnedAlgoCount, cudnnConvolutionFwdAlgoPerf_t* perfResults);
typedef cudnnStatus_t CUDAAPI tcudnnFindConvolutionForwardAlgorithmEx(cudnnHandle_t handle, const cudnnTensorDescriptor_t xDesc, const void* x, const cudnnFilterDescriptor_t wDesc, const void* w, const cudnnConvolutionDescriptor_t convDesc, const cudnnTensorDescriptor_t yDesc, void* y, const int requestedAlgoCount, int* returnedAlgoCount, cudnnConvolutionFwdAlgoPerf_t* perfResults, void* workSpace, size_t workSpaceSizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnIm2Col(cudnnHandle_t handle, const cudnnTensorDescriptor_t xDesc, const void* x, const cudnnFilterDescriptor_t wDesc, const cudnnConvolutionDescriptor_t convDesc, void* colBuffer);
typedef cudnnStatus_t CUDAAPI tcudnnReorderFilterAndBias(cudnnHandle_t handle, const cudnnFilterDescriptor_t filterDesc, cudnnReorderType_t reorderType, const void* filterData, void* reorderedFilterData, int reorderBias, const void* biasData, void* reorderedBiasData);
typedef cudnnStatus_t CUDAAPI tcudnnGetConvolutionForwardWorkspaceSize(cudnnHandle_t handle, const cudnnTensorDescriptor_t xDesc, const cudnnFilterDescriptor_t wDesc, const cudnnConvolutionDescriptor_t convDesc, const cudnnTensorDescriptor_t yDesc, cudnnConvolutionFwdAlgo_t algo, size_t* sizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnConvolutionForward(cudnnHandle_t handle, const void* alpha, const cudnnTensorDescriptor_t xDesc, const void* x, const cudnnFilterDescriptor_t wDesc, const void* w, const cudnnConvolutionDescriptor_t convDesc, cudnnConvolutionFwdAlgo_t algo, void* workSpace, size_t workSpaceSizeInBytes, const void* beta, const cudnnTensorDescriptor_t yDesc, void* y);
typedef cudnnStatus_t CUDAAPI tcudnnConvolutionBiasActivationForward(cudnnHandle_t handle, const void* alpha1, const cudnnTensorDescriptor_t xDesc, const void* x, const cudnnFilterDescriptor_t wDesc, const void* w, const cudnnConvolutionDescriptor_t convDesc, cudnnConvolutionFwdAlgo_t algo, void* workSpace, size_t workSpaceSizeInBytes, const void* alpha2, const cudnnTensorDescriptor_t zDesc, const void* z, const cudnnTensorDescriptor_t biasDesc, const void* bias, const cudnnActivationDescriptor_t activationDesc, const cudnnTensorDescriptor_t yDesc, void* y);
typedef cudnnStatus_t CUDAAPI tcudnnGetConvolutionBackwardDataAlgorithmMaxCount(cudnnHandle_t handle, int* count);
typedef cudnnStatus_t CUDAAPI tcudnnFindConvolutionBackwardDataAlgorithm(cudnnHandle_t handle, const cudnnFilterDescriptor_t wDesc, const cudnnTensorDescriptor_t dyDesc, const cudnnConvolutionDescriptor_t convDesc, const cudnnTensorDescriptor_t dxDesc, const int requestedAlgoCount, int* returnedAlgoCount, cudnnConvolutionBwdDataAlgoPerf_t* perfResults);
typedef cudnnStatus_t CUDAAPI tcudnnFindConvolutionBackwardDataAlgorithmEx(cudnnHandle_t handle, const cudnnFilterDescriptor_t wDesc, const void* w, const cudnnTensorDescriptor_t dyDesc, const void* dy, const cudnnConvolutionDescriptor_t convDesc, const cudnnTensorDescriptor_t dxDesc, void* dx, const int requestedAlgoCount, int* returnedAlgoCount, cudnnConvolutionBwdDataAlgoPerf_t* perfResults, void* workSpace, size_t workSpaceSizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnGetConvolutionBackwardDataAlgorithm_v7(cudnnHandle_t handle, const cudnnFilterDescriptor_t filterDesc, const cudnnTensorDescriptor_t diffDesc, const cudnnConvolutionDescriptor_t convDesc, const cudnnTensorDescriptor_t gradDesc, const int requestedAlgoCount, int* returnedAlgoCount, cudnnConvolutionBwdDataAlgoPerf_t* perfResults);
typedef cudnnStatus_t CUDAAPI tcudnnGetConvolutionBackwardDataWorkspaceSize(cudnnHandle_t handle, const cudnnFilterDescriptor_t wDesc, const cudnnTensorDescriptor_t dyDesc, const cudnnConvolutionDescriptor_t convDesc, const cudnnTensorDescriptor_t dxDesc, cudnnConvolutionBwdDataAlgo_t algo, size_t* sizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnConvolutionBackwardData(cudnnHandle_t handle, const void* alpha, const cudnnFilterDescriptor_t wDesc, const void* w, const cudnnTensorDescriptor_t dyDesc, const void* dy, const cudnnConvolutionDescriptor_t convDesc, cudnnConvolutionBwdDataAlgo_t algo, void* workSpace, size_t workSpaceSizeInBytes, const void* beta, const cudnnTensorDescriptor_t dxDesc, void* dx);
typedef cudnnStatus_t CUDAAPI tcudnnGetFoldedConvBackwardDataDescriptors(const cudnnHandle_t handle, const cudnnFilterDescriptor_t filterDesc, const cudnnTensorDescriptor_t diffDesc, const cudnnConvolutionDescriptor_t convDesc, const cudnnTensorDescriptor_t gradDesc, const cudnnTensorFormat_t transformFormat, cudnnFilterDescriptor_t foldedFilterDesc, cudnnTensorDescriptor_t paddedDiffDesc, cudnnConvolutionDescriptor_t foldedConvDesc, cudnnTensorDescriptor_t foldedGradDesc, cudnnTensorTransformDescriptor_t filterFoldTransDesc, cudnnTensorTransformDescriptor_t diffPadTransDesc, cudnnTensorTransformDescriptor_t gradFoldTransDesc, cudnnTensorTransformDescriptor_t gradUnfoldTransDesc);
typedef cudnnStatus_t CUDAAPI tcudnnCnnInferVersionCheck(void);
typedef cudnnStatus_t CUDAAPI tcudnnGetConvolutionBackwardFilterAlgorithmMaxCount(cudnnHandle_t handle, int* count);
typedef cudnnStatus_t CUDAAPI tcudnnFindConvolutionBackwardFilterAlgorithm(cudnnHandle_t handle, const cudnnTensorDescriptor_t xDesc, const cudnnTensorDescriptor_t dyDesc, const cudnnConvolutionDescriptor_t convDesc, const cudnnFilterDescriptor_t dwDesc, const int requestedAlgoCount, int* returnedAlgoCount, cudnnConvolutionBwdFilterAlgoPerf_t* perfResults);
typedef cudnnStatus_t CUDAAPI tcudnnFindConvolutionBackwardFilterAlgorithmEx(cudnnHandle_t handle, const cudnnTensorDescriptor_t xDesc, const void* x, const cudnnTensorDescriptor_t dyDesc, const void* y, const cudnnConvolutionDescriptor_t convDesc, const cudnnFilterDescriptor_t dwDesc, void* dw, const int requestedAlgoCount, int* returnedAlgoCount, cudnnConvolutionBwdFilterAlgoPerf_t* perfResults, void* workSpace, size_t workSpaceSizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnGetConvolutionBackwardFilterAlgorithm_v7(cudnnHandle_t handle, const cudnnTensorDescriptor_t srcDesc, const cudnnTensorDescriptor_t diffDesc, const cudnnConvolutionDescriptor_t convDesc, const cudnnFilterDescriptor_t gradDesc, const int requestedAlgoCount, int* returnedAlgoCount, cudnnConvolutionBwdFilterAlgoPerf_t* perfResults);
typedef cudnnStatus_t CUDAAPI tcudnnGetConvolutionBackwardFilterWorkspaceSize(cudnnHandle_t handle, const cudnnTensorDescriptor_t xDesc, const cudnnTensorDescriptor_t dyDesc, const cudnnConvolutionDescriptor_t convDesc, const cudnnFilterDescriptor_t gradDesc, cudnnConvolutionBwdFilterAlgo_t algo, size_t* sizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnConvolutionBackwardFilter(cudnnHandle_t handle, const void* alpha, const cudnnTensorDescriptor_t xDesc, const void* x, const cudnnTensorDescriptor_t dyDesc, const void* dy, const cudnnConvolutionDescriptor_t convDesc, cudnnConvolutionBwdFilterAlgo_t algo, void* workSpace, size_t workSpaceSizeInBytes, const void* beta, const cudnnFilterDescriptor_t dwDesc, void* dw);
typedef cudnnStatus_t CUDAAPI tcudnnConvolutionBackwardBias(cudnnHandle_t handle, const void* alpha, const cudnnTensorDescriptor_t dyDesc, const void* dy, const void* beta, const cudnnTensorDescriptor_t dbDesc, void* db);
typedef cudnnStatus_t CUDAAPI tcudnnCreateFusedOpsConstParamPack(cudnnFusedOpsConstParamPack_t* constPack, cudnnFusedOps_t ops);
typedef cudnnStatus_t CUDAAPI tcudnnDestroyFusedOpsConstParamPack(cudnnFusedOpsConstParamPack_t constPack);
typedef cudnnStatus_t CUDAAPI tcudnnSetFusedOpsConstParamPackAttribute(cudnnFusedOpsConstParamPack_t constPack, cudnnFusedOpsConstParamLabel_t paramLabel, const void* param);
typedef cudnnStatus_t CUDAAPI tcudnnGetFusedOpsConstParamPackAttribute(const cudnnFusedOpsConstParamPack_t constPack, cudnnFusedOpsConstParamLabel_t paramLabel, void* param, int* isNULL);
typedef cudnnStatus_t CUDAAPI tcudnnCreateFusedOpsVariantParamPack(cudnnFusedOpsVariantParamPack_t* varPack, cudnnFusedOps_t ops);
typedef cudnnStatus_t CUDAAPI tcudnnDestroyFusedOpsVariantParamPack(cudnnFusedOpsVariantParamPack_t varPack);
typedef cudnnStatus_t CUDAAPI tcudnnSetFusedOpsVariantParamPackAttribute(cudnnFusedOpsVariantParamPack_t varPack, cudnnFusedOpsVariantParamLabel_t paramLabel, void* ptr);
typedef cudnnStatus_t CUDAAPI tcudnnGetFusedOpsVariantParamPackAttribute(const cudnnFusedOpsVariantParamPack_t varPack, cudnnFusedOpsVariantParamLabel_t paramLabel, void* ptr);
typedef cudnnStatus_t CUDAAPI tcudnnCreateFusedOpsPlan(cudnnFusedOpsPlan_t* plan, cudnnFusedOps_t ops);
typedef cudnnStatus_t CUDAAPI tcudnnDestroyFusedOpsPlan(cudnnFusedOpsPlan_t plan);
typedef cudnnStatus_t CUDAAPI tcudnnMakeFusedOpsPlan(cudnnHandle_t handle, cudnnFusedOpsPlan_t plan, const cudnnFusedOpsConstParamPack_t constPack, size_t* workspaceSizeInBytes);
typedef cudnnStatus_t CUDAAPI tcudnnFusedOpsExecute(cudnnHandle_t handle, const cudnnFusedOpsPlan_t plan, cudnnFusedOpsVariantParamPack_t varPack);
typedef cudnnStatus_t CUDAAPI tcudnnCnnTrainVersionCheck(void);
typedef cudnnStatus_t CUDAAPI tcudnnBackendCreateDescriptor(cudnnBackendDescriptorType_t descriptorType, cudnnBackendDescriptor_t* descriptor);
typedef cudnnStatus_t CUDAAPI tcudnnBackendDestroyDescriptor(cudnnBackendDescriptor_t descriptor);
typedef cudnnStatus_t CUDAAPI tcudnnBackendInitialize(cudnnBackendDescriptor_t descriptor);
typedef cudnnStatus_t CUDAAPI tcudnnBackendFinalize(cudnnBackendDescriptor_t descriptor);
typedef cudnnStatus_t CUDAAPI tcudnnBackendSetAttribute(cudnnBackendDescriptor_t descriptor, cudnnBackendAttributeName_t attributeName, cudnnBackendAttributeType_t attributeType, int64_t elementCount, const void* arrayOfElements);
typedef cudnnStatus_t CUDAAPI tcudnnBackendGetAttribute(const cudnnBackendDescriptor_t descriptor, cudnnBackendAttributeName_t attributeName, cudnnBackendAttributeType_t attributeType, int64_t requestedElementCount, int64_t* elementCount, void* arrayOfElements);
typedef cudnnStatus_t CUDAAPI tcudnnBackendExecute(cudnnHandle_t handle, cudnnBackendDescriptor_t executionPlan, cudnnBackendDescriptor_t variantPack);

typedef CUresult CUDAAPI tcuGraphicsGLRegisterBuffer(CUgraphicsResource* pCudaResource, GLuint buffer, unsigned int Flags);
typedef CUresult CUDAAPI tcuGraphicsGLRegisterImage(CUgraphicsResource* pCudaResource, GLuint image, GLenum target, unsigned int Flags);
typedef CUresult CUDAAPI tcuGLGetDevices_v2(unsigned int* pCudaDeviceCount, CUdevice* pCudaDevices, unsigned int cudaDeviceCount, CUGLDeviceList deviceList);
typedef CUresult CUDAAPI tcuGLCtxCreate_v2(CUcontext* pCtx, unsigned int Flags, CUdevice device);
typedef CUresult CUDAAPI tcuGLInit(void);
typedef CUresult CUDAAPI tcuGLRegisterBufferObject(GLuint buffer);
typedef CUresult CUDAAPI tcuGLMapBufferObject_v2(CUdeviceptr* dptr, size_t* size, GLuint buffer);
typedef CUresult CUDAAPI tcuGLUnmapBufferObject(GLuint buffer);
typedef CUresult CUDAAPI tcuGLUnregisterBufferObject(GLuint buffer);
typedef CUresult CUDAAPI tcuGLSetBufferObjectMapFlags(GLuint buffer, unsigned int Flags);
typedef CUresult CUDAAPI tcuGLMapBufferObjectAsync_v2(CUdeviceptr* dptr, size_t* size, GLuint buffer, CUstream hStream);
typedef CUresult CUDAAPI tcuGLUnmapBufferObjectAsync(GLuint buffer, CUstream hStream);


/* Function declarations. */
extern tcuGetErrorString *cuGetErrorString;
extern tcuGetErrorName *cuGetErrorName;
extern tcuInit *cuInit;
extern tcuDriverGetVersion *cuDriverGetVersion;
extern tcuDeviceGet *cuDeviceGet;
extern tcuDeviceGetCount *cuDeviceGetCount;
extern tcuDeviceGetName *cuDeviceGetName;
extern tcuDeviceGetUuid *cuDeviceGetUuid;
extern tcuDeviceGetUuid_v2 *cuDeviceGetUuid_v2;
extern tcuDeviceGetLuid *cuDeviceGetLuid;
extern tcuDeviceTotalMem_v2 *cuDeviceTotalMem_v2;
extern tcuDeviceGetTexture1DLinearMaxWidth *cuDeviceGetTexture1DLinearMaxWidth;
extern tcuDeviceGetAttribute *cuDeviceGetAttribute;
extern tcuDeviceGetNvSciSyncAttributes *cuDeviceGetNvSciSyncAttributes;
extern tcuDeviceSetMemPool *cuDeviceSetMemPool;
extern tcuDeviceGetMemPool *cuDeviceGetMemPool;
extern tcuDeviceGetDefaultMemPool *cuDeviceGetDefaultMemPool;
extern tcuDeviceGetExecAffinitySupport *cuDeviceGetExecAffinitySupport;
extern tcuFlushGPUDirectRDMAWrites *cuFlushGPUDirectRDMAWrites;
extern tcuDeviceGetProperties *cuDeviceGetProperties;
extern tcuDeviceComputeCapability *cuDeviceComputeCapability;
extern tcuDevicePrimaryCtxRetain *cuDevicePrimaryCtxRetain;
extern tcuDevicePrimaryCtxRelease_v2 *cuDevicePrimaryCtxRelease_v2;
extern tcuDevicePrimaryCtxSetFlags_v2 *cuDevicePrimaryCtxSetFlags_v2;
extern tcuDevicePrimaryCtxGetState *cuDevicePrimaryCtxGetState;
extern tcuDevicePrimaryCtxReset_v2 *cuDevicePrimaryCtxReset_v2;
extern tcuCtxCreate_v2 *cuCtxCreate_v2;
extern tcuCtxCreate_v3 *cuCtxCreate_v3;
extern tcuCtxDestroy_v2 *cuCtxDestroy_v2;
extern tcuCtxPushCurrent_v2 *cuCtxPushCurrent_v2;
extern tcuCtxPopCurrent_v2 *cuCtxPopCurrent_v2;
extern tcuCtxSetCurrent *cuCtxSetCurrent;
extern tcuCtxGetCurrent *cuCtxGetCurrent;
extern tcuCtxGetDevice *cuCtxGetDevice;
extern tcuCtxGetFlags *cuCtxGetFlags;
extern tcuCtxSetFlags *cuCtxSetFlags;
extern tcuCtxGetId *cuCtxGetId;
extern tcuCtxSynchronize *cuCtxSynchronize;
extern tcuCtxSetLimit *cuCtxSetLimit;
extern tcuCtxGetLimit *cuCtxGetLimit;
extern tcuCtxGetCacheConfig *cuCtxGetCacheConfig;
extern tcuCtxSetCacheConfig *cuCtxSetCacheConfig;
extern tcuCtxGetSharedMemConfig *cuCtxGetSharedMemConfig;
extern tcuCtxSetSharedMemConfig *cuCtxSetSharedMemConfig;
extern tcuCtxGetApiVersion *cuCtxGetApiVersion;
extern tcuCtxGetStreamPriorityRange *cuCtxGetStreamPriorityRange;
extern tcuCtxResetPersistingL2Cache *cuCtxResetPersistingL2Cache;
extern tcuCtxGetExecAffinity *cuCtxGetExecAffinity;
extern tcuCtxAttach *cuCtxAttach;
extern tcuCtxDetach *cuCtxDetach;
extern tcuModuleLoad *cuModuleLoad;
extern tcuModuleLoadData *cuModuleLoadData;
extern tcuModuleLoadDataEx *cuModuleLoadDataEx;
extern tcuModuleLoadFatBinary *cuModuleLoadFatBinary;
extern tcuModuleUnload *cuModuleUnload;
extern tcuModuleGetLoadingMode *cuModuleGetLoadingMode;
extern tcuModuleGetFunction *cuModuleGetFunction;
extern tcuModuleGetGlobal_v2 *cuModuleGetGlobal_v2;
extern tcuLinkCreate_v2 *cuLinkCreate_v2;
extern tcuLinkAddData_v2 *cuLinkAddData_v2;
extern tcuLinkAddFile_v2 *cuLinkAddFile_v2;
extern tcuLinkComplete *cuLinkComplete;
extern tcuLinkDestroy *cuLinkDestroy;
extern tcuModuleGetTexRef *cuModuleGetTexRef;
extern tcuModuleGetSurfRef *cuModuleGetSurfRef;
extern tcuLibraryLoadData *cuLibraryLoadData;
extern tcuLibraryLoadFromFile *cuLibraryLoadFromFile;
extern tcuLibraryUnload *cuLibraryUnload;
extern tcuLibraryGetKernel *cuLibraryGetKernel;
extern tcuLibraryGetModule *cuLibraryGetModule;
extern tcuKernelGetFunction *cuKernelGetFunction;
extern tcuLibraryGetGlobal *cuLibraryGetGlobal;
extern tcuLibraryGetManaged *cuLibraryGetManaged;
extern tcuLibraryGetUnifiedFunction *cuLibraryGetUnifiedFunction;
extern tcuKernelGetAttribute *cuKernelGetAttribute;
extern tcuKernelSetAttribute *cuKernelSetAttribute;
extern tcuKernelSetCacheConfig *cuKernelSetCacheConfig;
extern tcuMemGetInfo_v2 *cuMemGetInfo_v2;
extern tcuMemAlloc_v2 *cuMemAlloc_v2;
extern tcuMemAllocPitch_v2 *cuMemAllocPitch_v2;
extern tcuMemFree_v2 *cuMemFree_v2;
extern tcuMemGetAddressRange_v2 *cuMemGetAddressRange_v2;
extern tcuMemAllocHost_v2 *cuMemAllocHost_v2;
extern tcuMemFreeHost *cuMemFreeHost;
extern tcuMemHostAlloc *cuMemHostAlloc;
extern tcuMemHostGetDevicePointer_v2 *cuMemHostGetDevicePointer_v2;
extern tcuMemHostGetFlags *cuMemHostGetFlags;
extern tcuMemAllocManaged *cuMemAllocManaged;
extern tcuDeviceGetByPCIBusId *cuDeviceGetByPCIBusId;
extern tcuDeviceGetPCIBusId *cuDeviceGetPCIBusId;
extern tcuIpcGetEventHandle *cuIpcGetEventHandle;
extern tcuIpcOpenEventHandle *cuIpcOpenEventHandle;
extern tcuIpcGetMemHandle *cuIpcGetMemHandle;
extern tcuIpcOpenMemHandle_v2 *cuIpcOpenMemHandle_v2;
extern tcuIpcCloseMemHandle *cuIpcCloseMemHandle;
extern tcuMemHostRegister_v2 *cuMemHostRegister_v2;
extern tcuMemHostUnregister *cuMemHostUnregister;
extern tcuMemcpy *cuMemcpy;
extern tcuMemcpyPeer *cuMemcpyPeer;
extern tcuMemcpyHtoD_v2 *cuMemcpyHtoD_v2;
extern tcuMemcpyDtoH_v2 *cuMemcpyDtoH_v2;
extern tcuMemcpyDtoD_v2 *cuMemcpyDtoD_v2;
extern tcuMemcpyDtoA_v2 *cuMemcpyDtoA_v2;
extern tcuMemcpyAtoD_v2 *cuMemcpyAtoD_v2;
extern tcuMemcpyHtoA_v2 *cuMemcpyHtoA_v2;
extern tcuMemcpyAtoH_v2 *cuMemcpyAtoH_v2;
extern tcuMemcpyAtoA_v2 *cuMemcpyAtoA_v2;
extern tcuMemcpy2D_v2 *cuMemcpy2D_v2;
extern tcuMemcpy2DUnaligned_v2 *cuMemcpy2DUnaligned_v2;
extern tcuMemcpy3D_v2 *cuMemcpy3D_v2;
extern tcuMemcpy3DPeer *cuMemcpy3DPeer;
extern tcuMemcpyAsync *cuMemcpyAsync;
extern tcuMemcpyPeerAsync *cuMemcpyPeerAsync;
extern tcuMemcpyHtoDAsync_v2 *cuMemcpyHtoDAsync_v2;
extern tcuMemcpyDtoHAsync_v2 *cuMemcpyDtoHAsync_v2;
extern tcuMemcpyDtoDAsync_v2 *cuMemcpyDtoDAsync_v2;
extern tcuMemcpyHtoAAsync_v2 *cuMemcpyHtoAAsync_v2;
extern tcuMemcpyAtoHAsync_v2 *cuMemcpyAtoHAsync_v2;
extern tcuMemcpy2DAsync_v2 *cuMemcpy2DAsync_v2;
extern tcuMemcpy3DAsync_v2 *cuMemcpy3DAsync_v2;
extern tcuMemcpy3DPeerAsync *cuMemcpy3DPeerAsync;
extern tcuMemsetD8_v2 *cuMemsetD8_v2;
extern tcuMemsetD16_v2 *cuMemsetD16_v2;
extern tcuMemsetD32_v2 *cuMemsetD32_v2;
extern tcuMemsetD2D8_v2 *cuMemsetD2D8_v2;
extern tcuMemsetD2D16_v2 *cuMemsetD2D16_v2;
extern tcuMemsetD2D32_v2 *cuMemsetD2D32_v2;
extern tcuMemsetD8Async *cuMemsetD8Async;
extern tcuMemsetD16Async *cuMemsetD16Async;
extern tcuMemsetD32Async *cuMemsetD32Async;
extern tcuMemsetD2D8Async *cuMemsetD2D8Async;
extern tcuMemsetD2D16Async *cuMemsetD2D16Async;
extern tcuMemsetD2D32Async *cuMemsetD2D32Async;
extern tcuArrayCreate_v2 *cuArrayCreate_v2;
extern tcuArrayGetDescriptor_v2 *cuArrayGetDescriptor_v2;
extern tcuArrayGetSparseProperties *cuArrayGetSparseProperties;
extern tcuMipmappedArrayGetSparseProperties *cuMipmappedArrayGetSparseProperties;
extern tcuArrayGetMemoryRequirements *cuArrayGetMemoryRequirements;
extern tcuMipmappedArrayGetMemoryRequirements *cuMipmappedArrayGetMemoryRequirements;
extern tcuArrayGetPlane *cuArrayGetPlane;
extern tcuArrayDestroy *cuArrayDestroy;
extern tcuArray3DCreate_v2 *cuArray3DCreate_v2;
extern tcuArray3DGetDescriptor_v2 *cuArray3DGetDescriptor_v2;
extern tcuMipmappedArrayCreate *cuMipmappedArrayCreate;
extern tcuMipmappedArrayGetLevel *cuMipmappedArrayGetLevel;
extern tcuMipmappedArrayDestroy *cuMipmappedArrayDestroy;
extern tcuMemGetHandleForAddressRange *cuMemGetHandleForAddressRange;
extern tcuMemAddressReserve *cuMemAddressReserve;
extern tcuMemAddressFree *cuMemAddressFree;
extern tcuMemCreate *cuMemCreate;
extern tcuMemRelease *cuMemRelease;
extern tcuMemMap *cuMemMap;
extern tcuMemMapArrayAsync *cuMemMapArrayAsync;
extern tcuMemUnmap *cuMemUnmap;
extern tcuMemSetAccess *cuMemSetAccess;
extern tcuMemGetAccess *cuMemGetAccess;
extern tcuMemExportToShareableHandle *cuMemExportToShareableHandle;
extern tcuMemImportFromShareableHandle *cuMemImportFromShareableHandle;
extern tcuMemGetAllocationGranularity *cuMemGetAllocationGranularity;
extern tcuMemGetAllocationPropertiesFromHandle *cuMemGetAllocationPropertiesFromHandle;
extern tcuMemRetainAllocationHandle *cuMemRetainAllocationHandle;
extern tcuMemFreeAsync *cuMemFreeAsync;
extern tcuMemAllocAsync *cuMemAllocAsync;
extern tcuMemPoolTrimTo *cuMemPoolTrimTo;
extern tcuMemPoolSetAttribute *cuMemPoolSetAttribute;
extern tcuMemPoolGetAttribute *cuMemPoolGetAttribute;
extern tcuMemPoolSetAccess *cuMemPoolSetAccess;
extern tcuMemPoolGetAccess *cuMemPoolGetAccess;
extern tcuMemPoolCreate *cuMemPoolCreate;
extern tcuMemPoolDestroy *cuMemPoolDestroy;
extern tcuMemAllocFromPoolAsync *cuMemAllocFromPoolAsync;
extern tcuMemPoolExportToShareableHandle *cuMemPoolExportToShareableHandle;
extern tcuMemPoolImportFromShareableHandle *cuMemPoolImportFromShareableHandle;
extern tcuMemPoolExportPointer *cuMemPoolExportPointer;
extern tcuMemPoolImportPointer *cuMemPoolImportPointer;
extern tcuMulticastCreate *cuMulticastCreate;
extern tcuMulticastAddDevice *cuMulticastAddDevice;
extern tcuMulticastBindMem *cuMulticastBindMem;
extern tcuMulticastBindAddr *cuMulticastBindAddr;
extern tcuMulticastUnbind *cuMulticastUnbind;
extern tcuMulticastGetGranularity *cuMulticastGetGranularity;
extern tcuPointerGetAttribute *cuPointerGetAttribute;
extern tcuMemPrefetchAsync *cuMemPrefetchAsync;
extern tcuMemAdvise *cuMemAdvise;
extern tcuMemRangeGetAttribute *cuMemRangeGetAttribute;
extern tcuMemRangeGetAttributes *cuMemRangeGetAttributes;
extern tcuPointerSetAttribute *cuPointerSetAttribute;
extern tcuPointerGetAttributes *cuPointerGetAttributes;
extern tcuStreamCreate *cuStreamCreate;
extern tcuStreamCreateWithPriority *cuStreamCreateWithPriority;
extern tcuStreamGetPriority *cuStreamGetPriority;
extern tcuStreamGetFlags *cuStreamGetFlags;
extern tcuStreamGetId *cuStreamGetId;
extern tcuStreamGetCtx *cuStreamGetCtx;
extern tcuStreamWaitEvent *cuStreamWaitEvent;
extern tcuStreamAddCallback *cuStreamAddCallback;
extern tcuStreamBeginCapture_v2 *cuStreamBeginCapture_v2;
extern tcuThreadExchangeStreamCaptureMode *cuThreadExchangeStreamCaptureMode;
extern tcuStreamEndCapture *cuStreamEndCapture;
extern tcuStreamIsCapturing *cuStreamIsCapturing;
extern tcuStreamGetCaptureInfo_v2 *cuStreamGetCaptureInfo_v2;
extern tcuStreamUpdateCaptureDependencies *cuStreamUpdateCaptureDependencies;
extern tcuStreamAttachMemAsync *cuStreamAttachMemAsync;
extern tcuStreamQuery *cuStreamQuery;
extern tcuStreamSynchronize *cuStreamSynchronize;
extern tcuStreamDestroy_v2 *cuStreamDestroy_v2;
extern tcuStreamCopyAttributes *cuStreamCopyAttributes;
extern tcuStreamGetAttribute *cuStreamGetAttribute;
extern tcuStreamSetAttribute *cuStreamSetAttribute;
extern tcuEventCreate *cuEventCreate;
extern tcuEventRecord *cuEventRecord;
extern tcuEventRecordWithFlags *cuEventRecordWithFlags;
extern tcuEventQuery *cuEventQuery;
extern tcuEventSynchronize *cuEventSynchronize;
extern tcuEventDestroy_v2 *cuEventDestroy_v2;
extern tcuEventElapsedTime *cuEventElapsedTime;
extern tcuImportExternalMemory *cuImportExternalMemory;
extern tcuExternalMemoryGetMappedBuffer *cuExternalMemoryGetMappedBuffer;
extern tcuExternalMemoryGetMappedMipmappedArray *cuExternalMemoryGetMappedMipmappedArray;
extern tcuDestroyExternalMemory *cuDestroyExternalMemory;
extern tcuImportExternalSemaphore *cuImportExternalSemaphore;
extern tcuSignalExternalSemaphoresAsync *cuSignalExternalSemaphoresAsync;
extern tcuWaitExternalSemaphoresAsync *cuWaitExternalSemaphoresAsync;
extern tcuDestroyExternalSemaphore *cuDestroyExternalSemaphore;
extern tcuStreamWaitValue32_v2 *cuStreamWaitValue32_v2;
extern tcuStreamWaitValue64_v2 *cuStreamWaitValue64_v2;
extern tcuStreamWriteValue32_v2 *cuStreamWriteValue32_v2;
extern tcuStreamWriteValue64_v2 *cuStreamWriteValue64_v2;
extern tcuStreamBatchMemOp_v2 *cuStreamBatchMemOp_v2;
extern tcuFuncGetAttribute *cuFuncGetAttribute;
extern tcuFuncSetAttribute *cuFuncSetAttribute;
extern tcuFuncSetCacheConfig *cuFuncSetCacheConfig;
extern tcuFuncSetSharedMemConfig *cuFuncSetSharedMemConfig;
extern tcuFuncGetModule *cuFuncGetModule;
extern tcuLaunchKernel *cuLaunchKernel;
extern tcuLaunchKernelEx *cuLaunchKernelEx;
extern tcuLaunchCooperativeKernel *cuLaunchCooperativeKernel;
extern tcuLaunchCooperativeKernelMultiDevice *cuLaunchCooperativeKernelMultiDevice;
extern tcuLaunchHostFunc *cuLaunchHostFunc;
extern tcuFuncSetBlockShape *cuFuncSetBlockShape;
extern tcuFuncSetSharedSize *cuFuncSetSharedSize;
extern tcuParamSetSize *cuParamSetSize;
extern tcuParamSeti *cuParamSeti;
extern tcuParamSetf *cuParamSetf;
extern tcuParamSetv *cuParamSetv;
extern tcuLaunch *cuLaunch;
extern tcuLaunchGrid *cuLaunchGrid;
extern tcuLaunchGridAsync *cuLaunchGridAsync;
extern tcuParamSetTexRef *cuParamSetTexRef;
extern tcuGraphCreate *cuGraphCreate;
extern tcuGraphAddKernelNode_v2 *cuGraphAddKernelNode_v2;
extern tcuGraphKernelNodeGetParams_v2 *cuGraphKernelNodeGetParams_v2;
extern tcuGraphKernelNodeSetParams_v2 *cuGraphKernelNodeSetParams_v2;
extern tcuGraphAddMemcpyNode *cuGraphAddMemcpyNode;
extern tcuGraphMemcpyNodeGetParams *cuGraphMemcpyNodeGetParams;
extern tcuGraphMemcpyNodeSetParams *cuGraphMemcpyNodeSetParams;
extern tcuGraphAddMemsetNode *cuGraphAddMemsetNode;
extern tcuGraphMemsetNodeGetParams *cuGraphMemsetNodeGetParams;
extern tcuGraphMemsetNodeSetParams *cuGraphMemsetNodeSetParams;
extern tcuGraphAddHostNode *cuGraphAddHostNode;
extern tcuGraphHostNodeGetParams *cuGraphHostNodeGetParams;
extern tcuGraphHostNodeSetParams *cuGraphHostNodeSetParams;
extern tcuGraphAddChildGraphNode *cuGraphAddChildGraphNode;
extern tcuGraphChildGraphNodeGetGraph *cuGraphChildGraphNodeGetGraph;
extern tcuGraphAddEmptyNode *cuGraphAddEmptyNode;
extern tcuGraphAddEventRecordNode *cuGraphAddEventRecordNode;
extern tcuGraphEventRecordNodeGetEvent *cuGraphEventRecordNodeGetEvent;
extern tcuGraphEventRecordNodeSetEvent *cuGraphEventRecordNodeSetEvent;
extern tcuGraphAddEventWaitNode *cuGraphAddEventWaitNode;
extern tcuGraphEventWaitNodeGetEvent *cuGraphEventWaitNodeGetEvent;
extern tcuGraphEventWaitNodeSetEvent *cuGraphEventWaitNodeSetEvent;
extern tcuGraphAddExternalSemaphoresSignalNode *cuGraphAddExternalSemaphoresSignalNode;
extern tcuGraphExternalSemaphoresSignalNodeGetParams *cuGraphExternalSemaphoresSignalNodeGetParams;
extern tcuGraphExternalSemaphoresSignalNodeSetParams *cuGraphExternalSemaphoresSignalNodeSetParams;
extern tcuGraphAddExternalSemaphoresWaitNode *cuGraphAddExternalSemaphoresWaitNode;
extern tcuGraphExternalSemaphoresWaitNodeGetParams *cuGraphExternalSemaphoresWaitNodeGetParams;
extern tcuGraphExternalSemaphoresWaitNodeSetParams *cuGraphExternalSemaphoresWaitNodeSetParams;
extern tcuGraphAddBatchMemOpNode *cuGraphAddBatchMemOpNode;
extern tcuGraphBatchMemOpNodeGetParams *cuGraphBatchMemOpNodeGetParams;
extern tcuGraphBatchMemOpNodeSetParams *cuGraphBatchMemOpNodeSetParams;
extern tcuGraphExecBatchMemOpNodeSetParams *cuGraphExecBatchMemOpNodeSetParams;
extern tcuGraphAddMemAllocNode *cuGraphAddMemAllocNode;
extern tcuGraphMemAllocNodeGetParams *cuGraphMemAllocNodeGetParams;
extern tcuGraphAddMemFreeNode *cuGraphAddMemFreeNode;
extern tcuGraphMemFreeNodeGetParams *cuGraphMemFreeNodeGetParams;
extern tcuDeviceGraphMemTrim *cuDeviceGraphMemTrim;
extern tcuDeviceGetGraphMemAttribute *cuDeviceGetGraphMemAttribute;
extern tcuDeviceSetGraphMemAttribute *cuDeviceSetGraphMemAttribute;
extern tcuGraphClone *cuGraphClone;
extern tcuGraphNodeFindInClone *cuGraphNodeFindInClone;
extern tcuGraphNodeGetType *cuGraphNodeGetType;
extern tcuGraphGetNodes *cuGraphGetNodes;
extern tcuGraphGetRootNodes *cuGraphGetRootNodes;
extern tcuGraphGetEdges *cuGraphGetEdges;
extern tcuGraphNodeGetDependencies *cuGraphNodeGetDependencies;
extern tcuGraphNodeGetDependentNodes *cuGraphNodeGetDependentNodes;
extern tcuGraphAddDependencies *cuGraphAddDependencies;
extern tcuGraphRemoveDependencies *cuGraphRemoveDependencies;
extern tcuGraphDestroyNode *cuGraphDestroyNode;
extern tcuGraphInstantiateWithFlags *cuGraphInstantiateWithFlags;
extern tcuGraphInstantiateWithParams *cuGraphInstantiateWithParams;
extern tcuGraphExecGetFlags *cuGraphExecGetFlags;
extern tcuGraphExecKernelNodeSetParams_v2 *cuGraphExecKernelNodeSetParams_v2;
extern tcuGraphExecMemcpyNodeSetParams *cuGraphExecMemcpyNodeSetParams;
extern tcuGraphExecMemsetNodeSetParams *cuGraphExecMemsetNodeSetParams;
extern tcuGraphExecHostNodeSetParams *cuGraphExecHostNodeSetParams;
extern tcuGraphExecChildGraphNodeSetParams *cuGraphExecChildGraphNodeSetParams;
extern tcuGraphExecEventRecordNodeSetEvent *cuGraphExecEventRecordNodeSetEvent;
extern tcuGraphExecEventWaitNodeSetEvent *cuGraphExecEventWaitNodeSetEvent;
extern tcuGraphExecExternalSemaphoresSignalNodeSetParams *cuGraphExecExternalSemaphoresSignalNodeSetParams;
extern tcuGraphExecExternalSemaphoresWaitNodeSetParams *cuGraphExecExternalSemaphoresWaitNodeSetParams;
extern tcuGraphNodeSetEnabled *cuGraphNodeSetEnabled;
extern tcuGraphNodeGetEnabled *cuGraphNodeGetEnabled;
extern tcuGraphUpload *cuGraphUpload;
extern tcuGraphLaunch *cuGraphLaunch;
extern tcuGraphExecDestroy *cuGraphExecDestroy;
extern tcuGraphDestroy *cuGraphDestroy;
extern tcuGraphExecUpdate_v2 *cuGraphExecUpdate_v2;
extern tcuGraphKernelNodeCopyAttributes *cuGraphKernelNodeCopyAttributes;
extern tcuGraphKernelNodeGetAttribute *cuGraphKernelNodeGetAttribute;
extern tcuGraphKernelNodeSetAttribute *cuGraphKernelNodeSetAttribute;
extern tcuGraphDebugDotPrint *cuGraphDebugDotPrint;
extern tcuUserObjectCreate *cuUserObjectCreate;
extern tcuUserObjectRetain *cuUserObjectRetain;
extern tcuUserObjectRelease *cuUserObjectRelease;
extern tcuGraphRetainUserObject *cuGraphRetainUserObject;
extern tcuGraphReleaseUserObject *cuGraphReleaseUserObject;
extern tcuOccupancyMaxActiveBlocksPerMultiprocessor *cuOccupancyMaxActiveBlocksPerMultiprocessor;
extern tcuOccupancyMaxActiveBlocksPerMultiprocessorWithFlags *cuOccupancyMaxActiveBlocksPerMultiprocessorWithFlags;
extern tcuOccupancyMaxPotentialBlockSize *cuOccupancyMaxPotentialBlockSize;
extern tcuOccupancyMaxPotentialBlockSizeWithFlags *cuOccupancyMaxPotentialBlockSizeWithFlags;
extern tcuOccupancyAvailableDynamicSMemPerBlock *cuOccupancyAvailableDynamicSMemPerBlock;
extern tcuOccupancyMaxPotentialClusterSize *cuOccupancyMaxPotentialClusterSize;
extern tcuOccupancyMaxActiveClusters *cuOccupancyMaxActiveClusters;
extern tcuTexRefSetArray *cuTexRefSetArray;
extern tcuTexRefSetMipmappedArray *cuTexRefSetMipmappedArray;
extern tcuTexRefSetAddress_v2 *cuTexRefSetAddress_v2;
extern tcuTexRefSetAddress2D_v3 *cuTexRefSetAddress2D_v3;
extern tcuTexRefSetFormat *cuTexRefSetFormat;
extern tcuTexRefSetAddressMode *cuTexRefSetAddressMode;
extern tcuTexRefSetFilterMode *cuTexRefSetFilterMode;
extern tcuTexRefSetMipmapFilterMode *cuTexRefSetMipmapFilterMode;
extern tcuTexRefSetMipmapLevelBias *cuTexRefSetMipmapLevelBias;
extern tcuTexRefSetMipmapLevelClamp *cuTexRefSetMipmapLevelClamp;
extern tcuTexRefSetMaxAnisotropy *cuTexRefSetMaxAnisotropy;
extern tcuTexRefSetBorderColor *cuTexRefSetBorderColor;
extern tcuTexRefSetFlags *cuTexRefSetFlags;
extern tcuTexRefGetAddress_v2 *cuTexRefGetAddress_v2;
extern tcuTexRefGetArray *cuTexRefGetArray;
extern tcuTexRefGetMipmappedArray *cuTexRefGetMipmappedArray;
extern tcuTexRefGetAddressMode *cuTexRefGetAddressMode;
extern tcuTexRefGetFilterMode *cuTexRefGetFilterMode;
extern tcuTexRefGetFormat *cuTexRefGetFormat;
extern tcuTexRefGetMipmapFilterMode *cuTexRefGetMipmapFilterMode;
extern tcuTexRefGetMipmapLevelBias *cuTexRefGetMipmapLevelBias;
extern tcuTexRefGetMipmapLevelClamp *cuTexRefGetMipmapLevelClamp;
extern tcuTexRefGetMaxAnisotropy *cuTexRefGetMaxAnisotropy;
extern tcuTexRefGetBorderColor *cuTexRefGetBorderColor;
extern tcuTexRefGetFlags *cuTexRefGetFlags;
extern tcuTexRefCreate *cuTexRefCreate;
extern tcuTexRefDestroy *cuTexRefDestroy;
extern tcuSurfRefSetArray *cuSurfRefSetArray;
extern tcuSurfRefGetArray *cuSurfRefGetArray;
extern tcuTexObjectCreate *cuTexObjectCreate;
extern tcuTexObjectDestroy *cuTexObjectDestroy;
extern tcuTexObjectGetResourceDesc *cuTexObjectGetResourceDesc;
extern tcuTexObjectGetTextureDesc *cuTexObjectGetTextureDesc;
extern tcuTexObjectGetResourceViewDesc *cuTexObjectGetResourceViewDesc;
extern tcuSurfObjectCreate *cuSurfObjectCreate;
extern tcuSurfObjectDestroy *cuSurfObjectDestroy;
extern tcuSurfObjectGetResourceDesc *cuSurfObjectGetResourceDesc;
extern tcuTensorMapEncodeTiled *cuTensorMapEncodeTiled;
extern tcuTensorMapEncodeIm2col *cuTensorMapEncodeIm2col;
extern tcuTensorMapReplaceAddress *cuTensorMapReplaceAddress;
extern tcuDeviceCanAccessPeer *cuDeviceCanAccessPeer;
extern tcuCtxEnablePeerAccess *cuCtxEnablePeerAccess;
extern tcuCtxDisablePeerAccess *cuCtxDisablePeerAccess;
extern tcuDeviceGetP2PAttribute *cuDeviceGetP2PAttribute;
extern tcuGraphicsUnregisterResource *cuGraphicsUnregisterResource;
extern tcuGraphicsSubResourceGetMappedArray *cuGraphicsSubResourceGetMappedArray;
extern tcuGraphicsResourceGetMappedMipmappedArray *cuGraphicsResourceGetMappedMipmappedArray;
extern tcuGraphicsResourceGetMappedPointer_v2 *cuGraphicsResourceGetMappedPointer_v2;
extern tcuGraphicsResourceSetMapFlags_v2 *cuGraphicsResourceSetMapFlags_v2;
extern tcuGraphicsMapResources *cuGraphicsMapResources;
extern tcuGraphicsUnmapResources *cuGraphicsUnmapResources;
extern tcuGetProcAddress_v2 *cuGetProcAddress_v2;
extern tcuCoredumpGetAttribute *cuCoredumpGetAttribute;
extern tcuCoredumpGetAttributeGlobal *cuCoredumpGetAttributeGlobal;
extern tcuCoredumpSetAttribute *cuCoredumpSetAttribute;
extern tcuCoredumpSetAttributeGlobal *cuCoredumpSetAttributeGlobal;
extern tcuGetExportTable *cuGetExportTable;

extern tnvrtcGetErrorString *nvrtcGetErrorString;
extern tnvrtcVersion *nvrtcVersion;
extern tnvrtcGetNumSupportedArchs *nvrtcGetNumSupportedArchs;
extern tnvrtcGetSupportedArchs *nvrtcGetSupportedArchs;
extern tnvrtcCreateProgram *nvrtcCreateProgram;
extern tnvrtcDestroyProgram *nvrtcDestroyProgram;
extern tnvrtcCompileProgram *nvrtcCompileProgram;
extern tnvrtcGetPTXSize *nvrtcGetPTXSize;
extern tnvrtcGetPTX *nvrtcGetPTX;
extern tnvrtcGetCUBINSize *nvrtcGetCUBINSize;
extern tnvrtcGetCUBIN *nvrtcGetCUBIN;
extern tnvrtcGetNVVMSize *nvrtcGetNVVMSize;
extern tnvrtcGetNVVM *nvrtcGetNVVM;
extern tnvrtcGetLTOIRSize *nvrtcGetLTOIRSize;
extern tnvrtcGetLTOIR *nvrtcGetLTOIR;
extern tnvrtcGetOptiXIRSize *nvrtcGetOptiXIRSize;
extern tnvrtcGetOptiXIR *nvrtcGetOptiXIR;
extern tnvrtcGetProgramLogSize *nvrtcGetProgramLogSize;
extern tnvrtcGetProgramLog *nvrtcGetProgramLog;
extern tnvrtcAddNameExpression *nvrtcAddNameExpression;
extern tnvrtcGetLoweredName *nvrtcGetLoweredName;

extern tcudnnGetVersion *cudnnGetVersion;
extern tcudnnGetMaxDeviceVersion *cudnnGetMaxDeviceVersion;
extern tcudnnGetCudartVersion *cudnnGetCudartVersion;
extern tcudnnGetErrorString *cudnnGetErrorString;
extern tcudnnQueryRuntimeError *cudnnQueryRuntimeError;
extern tcudnnGetProperty *cudnnGetProperty;
extern tcudnnCreate *cudnnCreate;
extern tcudnnDestroy *cudnnDestroy;
extern tcudnnSetStream *cudnnSetStream;
extern tcudnnGetStream *cudnnGetStream;
extern tcudnnCreateTensorDescriptor *cudnnCreateTensorDescriptor;
extern tcudnnSetTensor4dDescriptor *cudnnSetTensor4dDescriptor;
extern tcudnnSetTensor4dDescriptorEx *cudnnSetTensor4dDescriptorEx;
extern tcudnnGetTensor4dDescriptor *cudnnGetTensor4dDescriptor;
extern tcudnnSetTensorNdDescriptor *cudnnSetTensorNdDescriptor;
extern tcudnnSetTensorNdDescriptorEx *cudnnSetTensorNdDescriptorEx;
extern tcudnnGetTensorNdDescriptor *cudnnGetTensorNdDescriptor;
extern tcudnnGetTensorSizeInBytes *cudnnGetTensorSizeInBytes;
extern tcudnnDestroyTensorDescriptor *cudnnDestroyTensorDescriptor;
extern tcudnnInitTransformDest *cudnnInitTransformDest;
extern tcudnnCreateTensorTransformDescriptor *cudnnCreateTensorTransformDescriptor;
extern tcudnnSetTensorTransformDescriptor *cudnnSetTensorTransformDescriptor;
extern tcudnnGetTensorTransformDescriptor *cudnnGetTensorTransformDescriptor;
extern tcudnnDestroyTensorTransformDescriptor *cudnnDestroyTensorTransformDescriptor;
extern tcudnnTransformTensor *cudnnTransformTensor;
extern tcudnnTransformTensorEx *cudnnTransformTensorEx;
extern tcudnnAddTensor *cudnnAddTensor;
extern tcudnnCreateOpTensorDescriptor *cudnnCreateOpTensorDescriptor;
extern tcudnnSetOpTensorDescriptor *cudnnSetOpTensorDescriptor;
extern tcudnnGetOpTensorDescriptor *cudnnGetOpTensorDescriptor;
extern tcudnnDestroyOpTensorDescriptor *cudnnDestroyOpTensorDescriptor;
extern tcudnnOpTensor *cudnnOpTensor;
extern tcudnnCreateReduceTensorDescriptor *cudnnCreateReduceTensorDescriptor;
extern tcudnnSetReduceTensorDescriptor *cudnnSetReduceTensorDescriptor;
extern tcudnnGetReduceTensorDescriptor *cudnnGetReduceTensorDescriptor;
extern tcudnnDestroyReduceTensorDescriptor *cudnnDestroyReduceTensorDescriptor;
extern tcudnnGetReductionIndicesSize *cudnnGetReductionIndicesSize;
extern tcudnnGetReductionWorkspaceSize *cudnnGetReductionWorkspaceSize;
extern tcudnnReduceTensor *cudnnReduceTensor;
extern tcudnnSetTensor *cudnnSetTensor;
extern tcudnnScaleTensor *cudnnScaleTensor;
extern tcudnnCreateFilterDescriptor *cudnnCreateFilterDescriptor;
extern tcudnnSetFilter4dDescriptor *cudnnSetFilter4dDescriptor;
extern tcudnnGetFilter4dDescriptor *cudnnGetFilter4dDescriptor;
extern tcudnnSetFilterNdDescriptor *cudnnSetFilterNdDescriptor;
extern tcudnnGetFilterNdDescriptor *cudnnGetFilterNdDescriptor;
extern tcudnnGetFilterSizeInBytes *cudnnGetFilterSizeInBytes;
extern tcudnnTransformFilter *cudnnTransformFilter;
extern tcudnnDestroyFilterDescriptor *cudnnDestroyFilterDescriptor;
extern tcudnnSoftmaxForward *cudnnSoftmaxForward;
extern tcudnnCreatePoolingDescriptor *cudnnCreatePoolingDescriptor;
extern tcudnnSetPooling2dDescriptor *cudnnSetPooling2dDescriptor;
extern tcudnnGetPooling2dDescriptor *cudnnGetPooling2dDescriptor;
extern tcudnnSetPoolingNdDescriptor *cudnnSetPoolingNdDescriptor;
extern tcudnnGetPoolingNdDescriptor *cudnnGetPoolingNdDescriptor;
extern tcudnnGetPoolingNdForwardOutputDim *cudnnGetPoolingNdForwardOutputDim;
extern tcudnnGetPooling2dForwardOutputDim *cudnnGetPooling2dForwardOutputDim;
extern tcudnnDestroyPoolingDescriptor *cudnnDestroyPoolingDescriptor;
extern tcudnnPoolingForward *cudnnPoolingForward;
extern tcudnnCreateActivationDescriptor *cudnnCreateActivationDescriptor;
extern tcudnnSetActivationDescriptor *cudnnSetActivationDescriptor;
extern tcudnnGetActivationDescriptor *cudnnGetActivationDescriptor;
extern tcudnnSetActivationDescriptorSwishBeta *cudnnSetActivationDescriptorSwishBeta;
extern tcudnnGetActivationDescriptorSwishBeta *cudnnGetActivationDescriptorSwishBeta;
extern tcudnnDestroyActivationDescriptor *cudnnDestroyActivationDescriptor;
extern tcudnnActivationForward *cudnnActivationForward;
extern tcudnnCreateLRNDescriptor *cudnnCreateLRNDescriptor;
extern tcudnnSetLRNDescriptor *cudnnSetLRNDescriptor;
extern tcudnnGetLRNDescriptor *cudnnGetLRNDescriptor;
extern tcudnnDestroyLRNDescriptor *cudnnDestroyLRNDescriptor;
extern tcudnnLRNCrossChannelForward *cudnnLRNCrossChannelForward;
extern tcudnnDivisiveNormalizationForward *cudnnDivisiveNormalizationForward;
extern tcudnnDeriveBNTensorDescriptor *cudnnDeriveBNTensorDescriptor;
extern tcudnnBatchNormalizationForwardInference *cudnnBatchNormalizationForwardInference;
extern tcudnnDeriveNormTensorDescriptor *cudnnDeriveNormTensorDescriptor;
extern tcudnnNormalizationForwardInference *cudnnNormalizationForwardInference;
extern tcudnnCreateSpatialTransformerDescriptor *cudnnCreateSpatialTransformerDescriptor;
extern tcudnnSetSpatialTransformerNdDescriptor *cudnnSetSpatialTransformerNdDescriptor;
extern tcudnnDestroySpatialTransformerDescriptor *cudnnDestroySpatialTransformerDescriptor;
extern tcudnnSpatialTfGridGeneratorForward *cudnnSpatialTfGridGeneratorForward;
extern tcudnnSpatialTfSamplerForward *cudnnSpatialTfSamplerForward;
extern tcudnnCreateDropoutDescriptor *cudnnCreateDropoutDescriptor;
extern tcudnnDestroyDropoutDescriptor *cudnnDestroyDropoutDescriptor;
extern tcudnnDropoutGetStatesSize *cudnnDropoutGetStatesSize;
extern tcudnnDropoutGetReserveSpaceSize *cudnnDropoutGetReserveSpaceSize;
extern tcudnnSetDropoutDescriptor *cudnnSetDropoutDescriptor;
extern tcudnnRestoreDropoutDescriptor *cudnnRestoreDropoutDescriptor;
extern tcudnnGetDropoutDescriptor *cudnnGetDropoutDescriptor;
extern tcudnnDropoutForward *cudnnDropoutForward;
extern tcudnnCreateAlgorithmDescriptor *cudnnCreateAlgorithmDescriptor;
extern tcudnnSetAlgorithmDescriptor *cudnnSetAlgorithmDescriptor;
extern tcudnnGetAlgorithmDescriptor *cudnnGetAlgorithmDescriptor;
extern tcudnnCopyAlgorithmDescriptor *cudnnCopyAlgorithmDescriptor;
extern tcudnnDestroyAlgorithmDescriptor *cudnnDestroyAlgorithmDescriptor;
extern tcudnnCreateAlgorithmPerformance *cudnnCreateAlgorithmPerformance;
extern tcudnnSetAlgorithmPerformance *cudnnSetAlgorithmPerformance;
extern tcudnnGetAlgorithmPerformance *cudnnGetAlgorithmPerformance;
extern tcudnnDestroyAlgorithmPerformance *cudnnDestroyAlgorithmPerformance;
extern tcudnnGetAlgorithmSpaceSize *cudnnGetAlgorithmSpaceSize;
extern tcudnnSaveAlgorithm *cudnnSaveAlgorithm;
extern tcudnnRestoreAlgorithm *cudnnRestoreAlgorithm;
extern tcudnnSetCallback *cudnnSetCallback;
extern tcudnnGetCallback *cudnnGetCallback;
extern tcudnnOpsInferVersionCheck *cudnnOpsInferVersionCheck;
extern tcudnnSoftmaxBackward *cudnnSoftmaxBackward;
extern tcudnnPoolingBackward *cudnnPoolingBackward;
extern tcudnnActivationBackward *cudnnActivationBackward;
extern tcudnnLRNCrossChannelBackward *cudnnLRNCrossChannelBackward;
extern tcudnnDivisiveNormalizationBackward *cudnnDivisiveNormalizationBackward;
extern tcudnnGetBatchNormalizationForwardTrainingExWorkspaceSize *cudnnGetBatchNormalizationForwardTrainingExWorkspaceSize;
extern tcudnnGetBatchNormalizationBackwardExWorkspaceSize *cudnnGetBatchNormalizationBackwardExWorkspaceSize;
extern tcudnnGetBatchNormalizationTrainingExReserveSpaceSize *cudnnGetBatchNormalizationTrainingExReserveSpaceSize;
extern tcudnnBatchNormalizationForwardTraining *cudnnBatchNormalizationForwardTraining;
extern tcudnnBatchNormalizationForwardTrainingEx *cudnnBatchNormalizationForwardTrainingEx;
extern tcudnnBatchNormalizationBackward *cudnnBatchNormalizationBackward;
extern tcudnnBatchNormalizationBackwardEx *cudnnBatchNormalizationBackwardEx;
extern tcudnnGetNormalizationForwardTrainingWorkspaceSize *cudnnGetNormalizationForwardTrainingWorkspaceSize;
extern tcudnnGetNormalizationBackwardWorkspaceSize *cudnnGetNormalizationBackwardWorkspaceSize;
extern tcudnnGetNormalizationTrainingReserveSpaceSize *cudnnGetNormalizationTrainingReserveSpaceSize;
extern tcudnnNormalizationForwardTraining *cudnnNormalizationForwardTraining;
extern tcudnnNormalizationBackward *cudnnNormalizationBackward;
extern tcudnnSpatialTfGridGeneratorBackward *cudnnSpatialTfGridGeneratorBackward;
extern tcudnnSpatialTfSamplerBackward *cudnnSpatialTfSamplerBackward;
extern tcudnnDropoutBackward *cudnnDropoutBackward;
extern tcudnnOpsTrainVersionCheck *cudnnOpsTrainVersionCheck;
extern tcudnnCreateRNNDescriptor *cudnnCreateRNNDescriptor;
extern tcudnnDestroyRNNDescriptor *cudnnDestroyRNNDescriptor;
extern tcudnnSetRNNDescriptor_v8 *cudnnSetRNNDescriptor_v8;
extern tcudnnGetRNNDescriptor_v8 *cudnnGetRNNDescriptor_v8;
extern tcudnnSetRNNDescriptor_v6 *cudnnSetRNNDescriptor_v6;
extern tcudnnGetRNNDescriptor_v6 *cudnnGetRNNDescriptor_v6;
extern tcudnnSetRNNMatrixMathType *cudnnSetRNNMatrixMathType;
extern tcudnnGetRNNMatrixMathType *cudnnGetRNNMatrixMathType;
extern tcudnnSetRNNBiasMode *cudnnSetRNNBiasMode;
extern tcudnnGetRNNBiasMode *cudnnGetRNNBiasMode;
extern tcudnnRNNSetClip_v8 *cudnnRNNSetClip_v8;
extern tcudnnRNNGetClip_v8 *cudnnRNNGetClip_v8;
extern tcudnnRNNSetClip *cudnnRNNSetClip;
extern tcudnnRNNGetClip *cudnnRNNGetClip;
extern tcudnnSetRNNProjectionLayers *cudnnSetRNNProjectionLayers;
extern tcudnnGetRNNProjectionLayers *cudnnGetRNNProjectionLayers;
extern tcudnnCreatePersistentRNNPlan *cudnnCreatePersistentRNNPlan;
extern tcudnnDestroyPersistentRNNPlan *cudnnDestroyPersistentRNNPlan;
extern tcudnnSetPersistentRNNPlan *cudnnSetPersistentRNNPlan;
extern tcudnnBuildRNNDynamic *cudnnBuildRNNDynamic;
extern tcudnnGetRNNWorkspaceSize *cudnnGetRNNWorkspaceSize;
extern tcudnnGetRNNTrainingReserveSize *cudnnGetRNNTrainingReserveSize;
extern tcudnnGetRNNTempSpaceSizes *cudnnGetRNNTempSpaceSizes;
extern tcudnnGetRNNParamsSize *cudnnGetRNNParamsSize;
extern tcudnnGetRNNWeightSpaceSize *cudnnGetRNNWeightSpaceSize;
extern tcudnnGetRNNLinLayerMatrixParams *cudnnGetRNNLinLayerMatrixParams;
extern tcudnnGetRNNLinLayerBiasParams *cudnnGetRNNLinLayerBiasParams;
extern tcudnnGetRNNWeightParams *cudnnGetRNNWeightParams;
extern tcudnnRNNForwardInference *cudnnRNNForwardInference;
extern tcudnnSetRNNPaddingMode *cudnnSetRNNPaddingMode;
extern tcudnnGetRNNPaddingMode *cudnnGetRNNPaddingMode;
extern tcudnnCreateRNNDataDescriptor *cudnnCreateRNNDataDescriptor;
extern tcudnnDestroyRNNDataDescriptor *cudnnDestroyRNNDataDescriptor;
extern tcudnnSetRNNDataDescriptor *cudnnSetRNNDataDescriptor;
extern tcudnnGetRNNDataDescriptor *cudnnGetRNNDataDescriptor;
extern tcudnnRNNForwardInferenceEx *cudnnRNNForwardInferenceEx;
extern tcudnnRNNForward *cudnnRNNForward;
extern tcudnnSetRNNAlgorithmDescriptor *cudnnSetRNNAlgorithmDescriptor;
extern tcudnnGetRNNForwardInferenceAlgorithmMaxCount *cudnnGetRNNForwardInferenceAlgorithmMaxCount;
extern tcudnnFindRNNForwardInferenceAlgorithmEx *cudnnFindRNNForwardInferenceAlgorithmEx;
extern tcudnnCreateSeqDataDescriptor *cudnnCreateSeqDataDescriptor;
extern tcudnnDestroySeqDataDescriptor *cudnnDestroySeqDataDescriptor;
extern tcudnnSetSeqDataDescriptor *cudnnSetSeqDataDescriptor;
extern tcudnnGetSeqDataDescriptor *cudnnGetSeqDataDescriptor;
extern tcudnnCreateAttnDescriptor *cudnnCreateAttnDescriptor;
extern tcudnnDestroyAttnDescriptor *cudnnDestroyAttnDescriptor;
extern tcudnnSetAttnDescriptor *cudnnSetAttnDescriptor;
extern tcudnnGetAttnDescriptor *cudnnGetAttnDescriptor;
extern tcudnnGetMultiHeadAttnBuffers *cudnnGetMultiHeadAttnBuffers;
extern tcudnnGetMultiHeadAttnWeights *cudnnGetMultiHeadAttnWeights;
extern tcudnnMultiHeadAttnForward *cudnnMultiHeadAttnForward;
extern tcudnnAdvInferVersionCheck *cudnnAdvInferVersionCheck;
extern tcudnnRNNForwardTraining *cudnnRNNForwardTraining;
extern tcudnnRNNBackwardData *cudnnRNNBackwardData;
extern tcudnnRNNBackwardData_v8 *cudnnRNNBackwardData_v8;
extern tcudnnRNNBackwardWeights *cudnnRNNBackwardWeights;
extern tcudnnRNNBackwardWeights_v8 *cudnnRNNBackwardWeights_v8;
extern tcudnnRNNForwardTrainingEx *cudnnRNNForwardTrainingEx;
extern tcudnnRNNBackwardDataEx *cudnnRNNBackwardDataEx;
extern tcudnnRNNBackwardWeightsEx *cudnnRNNBackwardWeightsEx;
extern tcudnnGetRNNForwardTrainingAlgorithmMaxCount *cudnnGetRNNForwardTrainingAlgorithmMaxCount;
extern tcudnnFindRNNForwardTrainingAlgorithmEx *cudnnFindRNNForwardTrainingAlgorithmEx;
extern tcudnnGetRNNBackwardDataAlgorithmMaxCount *cudnnGetRNNBackwardDataAlgorithmMaxCount;
extern tcudnnFindRNNBackwardDataAlgorithmEx *cudnnFindRNNBackwardDataAlgorithmEx;
extern tcudnnGetRNNBackwardWeightsAlgorithmMaxCount *cudnnGetRNNBackwardWeightsAlgorithmMaxCount;
extern tcudnnFindRNNBackwardWeightsAlgorithmEx *cudnnFindRNNBackwardWeightsAlgorithmEx;
extern tcudnnMultiHeadAttnBackwardData *cudnnMultiHeadAttnBackwardData;
extern tcudnnMultiHeadAttnBackwardWeights *cudnnMultiHeadAttnBackwardWeights;
extern tcudnnCreateCTCLossDescriptor *cudnnCreateCTCLossDescriptor;
extern tcudnnSetCTCLossDescriptor *cudnnSetCTCLossDescriptor;
extern tcudnnSetCTCLossDescriptorEx *cudnnSetCTCLossDescriptorEx;
extern tcudnnSetCTCLossDescriptor_v8 *cudnnSetCTCLossDescriptor_v8;
extern tcudnnGetCTCLossDescriptor *cudnnGetCTCLossDescriptor;
extern tcudnnGetCTCLossDescriptorEx *cudnnGetCTCLossDescriptorEx;
extern tcudnnGetCTCLossDescriptor_v8 *cudnnGetCTCLossDescriptor_v8;
extern tcudnnDestroyCTCLossDescriptor *cudnnDestroyCTCLossDescriptor;
extern tcudnnCTCLoss *cudnnCTCLoss;
extern tcudnnCTCLoss_v8 *cudnnCTCLoss_v8;
extern tcudnnGetCTCLossWorkspaceSize *cudnnGetCTCLossWorkspaceSize;
extern tcudnnGetCTCLossWorkspaceSize_v8 *cudnnGetCTCLossWorkspaceSize_v8;
extern tcudnnAdvTrainVersionCheck *cudnnAdvTrainVersionCheck;
extern tcudnnCreateConvolutionDescriptor *cudnnCreateConvolutionDescriptor;
extern tcudnnDestroyConvolutionDescriptor *cudnnDestroyConvolutionDescriptor;
extern tcudnnSetConvolutionMathType *cudnnSetConvolutionMathType;
extern tcudnnGetConvolutionMathType *cudnnGetConvolutionMathType;
extern tcudnnSetConvolutionGroupCount *cudnnSetConvolutionGroupCount;
extern tcudnnGetConvolutionGroupCount *cudnnGetConvolutionGroupCount;
extern tcudnnSetConvolutionReorderType *cudnnSetConvolutionReorderType;
extern tcudnnGetConvolutionReorderType *cudnnGetConvolutionReorderType;
extern tcudnnSetConvolution2dDescriptor *cudnnSetConvolution2dDescriptor;
extern tcudnnGetConvolution2dDescriptor *cudnnGetConvolution2dDescriptor;
extern tcudnnSetConvolutionNdDescriptor *cudnnSetConvolutionNdDescriptor;
extern tcudnnGetConvolutionNdDescriptor *cudnnGetConvolutionNdDescriptor;
extern tcudnnGetConvolution2dForwardOutputDim *cudnnGetConvolution2dForwardOutputDim;
extern tcudnnGetConvolutionNdForwardOutputDim *cudnnGetConvolutionNdForwardOutputDim;
extern tcudnnGetConvolutionForwardAlgorithmMaxCount *cudnnGetConvolutionForwardAlgorithmMaxCount;
extern tcudnnGetConvolutionForwardAlgorithm_v7 *cudnnGetConvolutionForwardAlgorithm_v7;
extern tcudnnFindConvolutionForwardAlgorithm *cudnnFindConvolutionForwardAlgorithm;
extern tcudnnFindConvolutionForwardAlgorithmEx *cudnnFindConvolutionForwardAlgorithmEx;
extern tcudnnIm2Col *cudnnIm2Col;
extern tcudnnReorderFilterAndBias *cudnnReorderFilterAndBias;
extern tcudnnGetConvolutionForwardWorkspaceSize *cudnnGetConvolutionForwardWorkspaceSize;
extern tcudnnConvolutionForward *cudnnConvolutionForward;
extern tcudnnConvolutionBiasActivationForward *cudnnConvolutionBiasActivationForward;
extern tcudnnGetConvolutionBackwardDataAlgorithmMaxCount *cudnnGetConvolutionBackwardDataAlgorithmMaxCount;
extern tcudnnFindConvolutionBackwardDataAlgorithm *cudnnFindConvolutionBackwardDataAlgorithm;
extern tcudnnFindConvolutionBackwardDataAlgorithmEx *cudnnFindConvolutionBackwardDataAlgorithmEx;
extern tcudnnGetConvolutionBackwardDataAlgorithm_v7 *cudnnGetConvolutionBackwardDataAlgorithm_v7;
extern tcudnnGetConvolutionBackwardDataWorkspaceSize *cudnnGetConvolutionBackwardDataWorkspaceSize;
extern tcudnnConvolutionBackwardData *cudnnConvolutionBackwardData;
extern tcudnnGetFoldedConvBackwardDataDescriptors *cudnnGetFoldedConvBackwardDataDescriptors;
extern tcudnnCnnInferVersionCheck *cudnnCnnInferVersionCheck;
extern tcudnnGetConvolutionBackwardFilterAlgorithmMaxCount *cudnnGetConvolutionBackwardFilterAlgorithmMaxCount;
extern tcudnnFindConvolutionBackwardFilterAlgorithm *cudnnFindConvolutionBackwardFilterAlgorithm;
extern tcudnnFindConvolutionBackwardFilterAlgorithmEx *cudnnFindConvolutionBackwardFilterAlgorithmEx;
extern tcudnnGetConvolutionBackwardFilterAlgorithm_v7 *cudnnGetConvolutionBackwardFilterAlgorithm_v7;
extern tcudnnGetConvolutionBackwardFilterWorkspaceSize *cudnnGetConvolutionBackwardFilterWorkspaceSize;
extern tcudnnConvolutionBackwardFilter *cudnnConvolutionBackwardFilter;
extern tcudnnConvolutionBackwardBias *cudnnConvolutionBackwardBias;
extern tcudnnCreateFusedOpsConstParamPack *cudnnCreateFusedOpsConstParamPack;
extern tcudnnDestroyFusedOpsConstParamPack *cudnnDestroyFusedOpsConstParamPack;
extern tcudnnSetFusedOpsConstParamPackAttribute *cudnnSetFusedOpsConstParamPackAttribute;
extern tcudnnGetFusedOpsConstParamPackAttribute *cudnnGetFusedOpsConstParamPackAttribute;
extern tcudnnCreateFusedOpsVariantParamPack *cudnnCreateFusedOpsVariantParamPack;
extern tcudnnDestroyFusedOpsVariantParamPack *cudnnDestroyFusedOpsVariantParamPack;
extern tcudnnSetFusedOpsVariantParamPackAttribute *cudnnSetFusedOpsVariantParamPackAttribute;
extern tcudnnGetFusedOpsVariantParamPackAttribute *cudnnGetFusedOpsVariantParamPackAttribute;
extern tcudnnCreateFusedOpsPlan *cudnnCreateFusedOpsPlan;
extern tcudnnDestroyFusedOpsPlan *cudnnDestroyFusedOpsPlan;
extern tcudnnMakeFusedOpsPlan *cudnnMakeFusedOpsPlan;
extern tcudnnFusedOpsExecute *cudnnFusedOpsExecute;
extern tcudnnCnnTrainVersionCheck *cudnnCnnTrainVersionCheck;
extern tcudnnBackendCreateDescriptor *cudnnBackendCreateDescriptor;
extern tcudnnBackendDestroyDescriptor *cudnnBackendDestroyDescriptor;
extern tcudnnBackendInitialize *cudnnBackendInitialize;
extern tcudnnBackendFinalize *cudnnBackendFinalize;
extern tcudnnBackendSetAttribute *cudnnBackendSetAttribute;
extern tcudnnBackendGetAttribute *cudnnBackendGetAttribute;
extern tcudnnBackendExecute *cudnnBackendExecute;

extern tcuGraphicsGLRegisterBuffer *cuGraphicsGLRegisterBuffer;
extern tcuGraphicsGLRegisterImage *cuGraphicsGLRegisterImage;
extern tcuGLGetDevices_v2 *cuGLGetDevices_v2;
extern tcuGLCtxCreate_v2 *cuGLCtxCreate_v2;
extern tcuGLInit *cuGLInit;
extern tcuGLRegisterBufferObject *cuGLRegisterBufferObject;
extern tcuGLMapBufferObject_v2 *cuGLMapBufferObject_v2;
extern tcuGLUnmapBufferObject *cuGLUnmapBufferObject;
extern tcuGLUnregisterBufferObject *cuGLUnregisterBufferObject;
extern tcuGLSetBufferObjectMapFlags *cuGLSetBufferObjectMapFlags;
extern tcuGLMapBufferObjectAsync_v2 *cuGLMapBufferObjectAsync_v2;
extern tcuGLUnmapBufferObjectAsync *cuGLUnmapBufferObjectAsync;


enum {
  CUEW_SUCCESS = 0,
  CUEW_ERROR_OPEN_FAILED = -1,
  CUEW_ERROR_ATEXIT_FAILED = -2,
};

enum {
	CUEW_INIT_CUDA  = (1 << 0),
	CUEW_INIT_NVRTC = (1 << 1),
	CUEW_INIT_CUDNN = (1 << 2),
};

int cuewInit(cuuint32_t flags);
const char *cuewErrorString(CUresult result);
const char *cuewCompilerPath(void);
int cuewCompilerVersion(void);
int cuewNvrtcVersion(void);

#ifdef __cplusplus
}
#endif

#endif  /* __CUEW_H__ */
