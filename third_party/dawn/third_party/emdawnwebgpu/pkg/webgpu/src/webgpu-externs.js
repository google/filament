/*
 * Copyright 2025 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 *
 * WebGPU globals
 * Generated using https://github.com/kainino0x/webidl-to-closure-externs
 * against the spec's WebIDL: https://gpuweb.github.io/gpuweb/webgpu.idl
 */

/** @type {?GPU} */
Navigator.prototype.gpu;

/** @type {?GPU} */
WorkerNavigator.prototype.gpu;

const GPUBufferUsage = {};
/** @type {number} */
GPUBufferUsage.MAP_READ;
/** @type {number} */
GPUBufferUsage.MAP_WRITE;
/** @type {number} */
GPUBufferUsage.COPY_SRC;
/** @type {number} */
GPUBufferUsage.COPY_DST;
/** @type {number} */
GPUBufferUsage.INDEX;
/** @type {number} */
GPUBufferUsage.VERTEX;
/** @type {number} */
GPUBufferUsage.UNIFORM;
/** @type {number} */
GPUBufferUsage.STORAGE;
/** @type {number} */
GPUBufferUsage.INDIRECT;
/** @type {number} */
GPUBufferUsage.QUERY_RESOLVE;

const GPUMapMode = {};
/** @type {number} */
GPUMapMode.READ;
/** @type {number} */
GPUMapMode.WRITE;

const GPUTextureUsage = {};
/** @type {number} */
GPUTextureUsage.COPY_SRC;
/** @type {number} */
GPUTextureUsage.COPY_DST;
/** @type {number} */
GPUTextureUsage.TEXTURE_BINDING;
/** @type {number} */
GPUTextureUsage.STORAGE_BINDING;
/** @type {number} */
GPUTextureUsage.RENDER_ATTACHMENT;

const GPUShaderStage = {};
/** @type {number} */
GPUShaderStage.VERTEX;
/** @type {number} */
GPUShaderStage.FRAGMENT;
/** @type {number} */
GPUShaderStage.COMPUTE;

const GPUColorWrite = {};
/** @type {number} */
GPUColorWrite.RED;
/** @type {number} */
GPUColorWrite.GREEN;
/** @type {number} */
GPUColorWrite.BLUE;
/** @type {number} */
GPUColorWrite.ALPHA;
/** @type {number} */
GPUColorWrite.ALL;

/** @constructor */
function GPUSupportedLimits() {}
/** @type {number} */
GPUSupportedLimits.prototype.maxTextureDimension1D;
/** @type {number} */
GPUSupportedLimits.prototype.maxTextureDimension2D;
/** @type {number} */
GPUSupportedLimits.prototype.maxTextureDimension3D;
/** @type {number} */
GPUSupportedLimits.prototype.maxTextureArrayLayers;
/** @type {number} */
GPUSupportedLimits.prototype.maxBindGroups;
/** @type {number} */
GPUSupportedLimits.prototype.maxBindGroupsPlusVertexBuffers;
/** @type {number} */
GPUSupportedLimits.prototype.maxBindingsPerBindGroup;
/** @type {number} */
GPUSupportedLimits.prototype.maxDynamicUniformBuffersPerPipelineLayout;
/** @type {number} */
GPUSupportedLimits.prototype.maxDynamicStorageBuffersPerPipelineLayout;
/** @type {number} */
GPUSupportedLimits.prototype.maxSampledTexturesPerShaderStage;
/** @type {number} */
GPUSupportedLimits.prototype.maxSamplersPerShaderStage;
/** @type {number} */
GPUSupportedLimits.prototype.maxStorageBuffersPerShaderStage;
/** @type {number} */
GPUSupportedLimits.prototype.maxStorageBuffersInVertexStage;
/** @type {number} */
GPUSupportedLimits.prototype.maxStorageBuffersInFragmentStage;
/** @type {number} */
GPUSupportedLimits.prototype.maxStorageTexturesPerShaderStage;
/** @type {number} */
GPUSupportedLimits.prototype.maxStorageTexturesInVertexStage;
/** @type {number} */
GPUSupportedLimits.prototype.maxStorageTexturesInFragmentStage;
/** @type {number} */
GPUSupportedLimits.prototype.maxUniformBuffersPerShaderStage;
/** @type {number} */
GPUSupportedLimits.prototype.maxUniformBufferBindingSize;
/** @type {number} */
GPUSupportedLimits.prototype.maxStorageBufferBindingSize;
/** @type {number} */
GPUSupportedLimits.prototype.minUniformBufferOffsetAlignment;
/** @type {number} */
GPUSupportedLimits.prototype.minStorageBufferOffsetAlignment;
/** @type {number} */
GPUSupportedLimits.prototype.maxVertexBuffers;
/** @type {number} */
GPUSupportedLimits.prototype.maxBufferSize;
/** @type {number} */
GPUSupportedLimits.prototype.maxVertexAttributes;
/** @type {number} */
GPUSupportedLimits.prototype.maxVertexBufferArrayStride;
/** @type {number} */
GPUSupportedLimits.prototype.maxInterStageShaderComponents;
/** @type {number} */
GPUSupportedLimits.prototype.maxInterStageShaderVariables;
/** @type {number} */
GPUSupportedLimits.prototype.maxColorAttachments;
/** @type {number} */
GPUSupportedLimits.prototype.maxColorAttachmentBytesPerSample;
/** @type {number} */
GPUSupportedLimits.prototype.maxComputeWorkgroupStorageSize;
/** @type {number} */
GPUSupportedLimits.prototype.maxComputeInvocationsPerWorkgroup;
/** @type {number} */
GPUSupportedLimits.prototype.maxComputeWorkgroupSizeX;
/** @type {number} */
GPUSupportedLimits.prototype.maxComputeWorkgroupSizeY;
/** @type {number} */
GPUSupportedLimits.prototype.maxComputeWorkgroupSizeZ;
/** @type {number} */
GPUSupportedLimits.prototype.maxComputeWorkgroupsPerDimension;
/** @type {number} */
GPUSupportedLimits.prototype.maxImmediateSize;

/**
 * @constructor
 * @implements {Iterable}
 */
function GPUSupportedFeatures() {}
/** @type {number} */
GPUSupportedFeatures.prototype.size;
/** @return {!Iterable<string>} */
GPUSupportedFeatures.prototype.entries = function(...args) {};
/** @return {!Iterable<string>} */
GPUSupportedFeatures.prototype.keys = function(...args) {};
/** @return {!Iterable<string>} */
GPUSupportedFeatures.prototype.values = function(...args) {};
/** @return {undefined} */
GPUSupportedFeatures.prototype.forEach = function(...args) {};
/** @return {boolean} */
GPUSupportedFeatures.prototype.has = function(...args) {};
/** @return {!Iterator<string>} */
GPUSupportedFeatures.prototype[Symbol.iterator] = function() {};

/**
 * @constructor
 * @implements {Iterable}
 */
function WGSLLanguageFeatures() {}
/** @type {number} */
WGSLLanguageFeatures.prototype.size;
/** @return {!Iterable<string>} */
WGSLLanguageFeatures.prototype.entries = function(...args) {};
/** @return {!Iterable<string>} */
WGSLLanguageFeatures.prototype.keys = function(...args) {};
/** @return {!Iterable<string>} */
WGSLLanguageFeatures.prototype.values = function(...args) {};
/** @return {undefined} */
WGSLLanguageFeatures.prototype.forEach = function(...args) {};
/** @return {boolean} */
WGSLLanguageFeatures.prototype.has = function(...args) {};
/** @return {!Iterator<string>} */
WGSLLanguageFeatures.prototype[Symbol.iterator] = function() {};

/** @constructor */
function GPUAdapterInfo() {}
/** @type {string} */
GPUAdapterInfo.prototype.vendor;
/** @type {string} */
GPUAdapterInfo.prototype.architecture;
/** @type {string} */
GPUAdapterInfo.prototype.device;
/** @type {string} */
GPUAdapterInfo.prototype.description;
/** @type {number} */
GPUAdapterInfo.prototype.subgroupMinSize;
/** @type {number} */
GPUAdapterInfo.prototype.subgroupMaxSize;

/** @constructor */
function GPU() {}
/** @return {!Promise<?GPUAdapter>} */
GPU.prototype.requestAdapter = function(...args) {};
/** @return {string} */
GPU.prototype.getPreferredCanvasFormat = function(...args) {};
/** @type {!WGSLLanguageFeatures} */
GPU.prototype.wgslLanguageFeatures;

/** @constructor */
function GPUAdapter() {}
/** @type {!GPUSupportedFeatures} */
GPUAdapter.prototype.features;
/** @type {!GPUSupportedLimits} */
GPUAdapter.prototype.limits;
/** @type {boolean} */
GPUAdapter.prototype.isFallbackAdapter;
/** @return {!Promise<!GPUDevice>} */
GPUAdapter.prototype.requestDevice = function(...args) {};
/** @return {!Promise<!GPUAdapterInfo>} */
GPUAdapter.prototype.requestAdapterInfo = function(...args) {};
/** @type {!GPUAdapterInfo} */
GPUAdapter.prototype.info;

/** @constructor */
function GPUDevice() {}
/** @type {string} */
GPUDevice.prototype.label;
/** @type {!GPUSupportedFeatures} */
GPUDevice.prototype.features;
/** @type {!GPUSupportedLimits} */
GPUDevice.prototype.limits;
/** @type {!GPUQueue} */
GPUDevice.prototype.queue;
/** @return {undefined} */
GPUDevice.prototype.destroy = function(...args) {};
/** @return {!GPUBuffer} */
GPUDevice.prototype.createBuffer = function(...args) {};
/** @return {!GPUTexture} */
GPUDevice.prototype.createTexture = function(...args) {};
/** @return {!GPUSampler} */
GPUDevice.prototype.createSampler = function(...args) {};
/** @return {!GPUExternalTexture} */
GPUDevice.prototype.importExternalTexture = function(...args) {};
/** @return {!GPUBindGroupLayout} */
GPUDevice.prototype.createBindGroupLayout = function(...args) {};
/** @return {!GPUPipelineLayout} */
GPUDevice.prototype.createPipelineLayout = function(...args) {};
/** @return {!GPUBindGroup} */
GPUDevice.prototype.createBindGroup = function(...args) {};
/** @return {!GPUShaderModule} */
GPUDevice.prototype.createShaderModule = function(...args) {};
/** @return {!GPUComputePipeline} */
GPUDevice.prototype.createComputePipeline = function(...args) {};
/** @return {!GPURenderPipeline} */
GPUDevice.prototype.createRenderPipeline = function(...args) {};
/** @return {!Promise<!GPUComputePipeline>} */
GPUDevice.prototype.createComputePipelineAsync = function(...args) {};
/** @return {!Promise<!GPURenderPipeline>} */
GPUDevice.prototype.createRenderPipelineAsync = function(...args) {};
/** @return {!GPUCommandEncoder} */
GPUDevice.prototype.createCommandEncoder = function(...args) {};
/** @return {!GPURenderBundleEncoder} */
GPUDevice.prototype.createRenderBundleEncoder = function(...args) {};
/** @return {!GPUQuerySet} */
GPUDevice.prototype.createQuerySet = function(...args) {};
/** @type {!Promise<!GPUDeviceLostInfo>} */
GPUDevice.prototype.lost;
/** @return {undefined} */
GPUDevice.prototype.pushErrorScope = function(...args) {};
/** @return {!Promise<?GPUError>} */
GPUDevice.prototype.popErrorScope = function(...args) {};
/** @type {!Function} */
GPUDevice.prototype.onuncapturederror;
/** @type {!GPUAdapterInfo} */
GPUDevice.prototype.adapterInfo;

/** @constructor */
function GPUBuffer() {}
/** @type {string} */
GPUBuffer.prototype.label;
/** @type {number} */
GPUBuffer.prototype.size;
/** @type {number} */
GPUBuffer.prototype.usage;
/** @type {string} */
GPUBuffer.prototype.mapState;
/** @return {!Promise<undefined>} */
GPUBuffer.prototype.mapAsync = function(...args) {};
/** @return {!ArrayBuffer} */
GPUBuffer.prototype.getMappedRange = function(...args) {};
/** @return {undefined} */
GPUBuffer.prototype.unmap = function(...args) {};
/** @return {undefined} */
GPUBuffer.prototype.destroy = function(...args) {};

/** @constructor */
function GPUTexture() {}
/** @type {string} */
GPUTexture.prototype.label;
/** @return {!GPUTextureView} */
GPUTexture.prototype.createView = function(...args) {};
/** @return {undefined} */
GPUTexture.prototype.destroy = function(...args) {};
/** @type {number} */
GPUTexture.prototype.width;
/** @type {number} */
GPUTexture.prototype.height;
/** @type {number} */
GPUTexture.prototype.depthOrArrayLayers;
/** @type {number} */
GPUTexture.prototype.mipLevelCount;
/** @type {number} */
GPUTexture.prototype.sampleCount;
/** @type {string} */
GPUTexture.prototype.dimension;
/** @type {string} */
GPUTexture.prototype.format;
/** @type {number} */
GPUTexture.prototype.usage;
/** @type {string | undefined} */
GPUTexture.prototype.textureBindingViewDimension;

/** @constructor */
function GPUTextureView() {}
/** @type {string} */
GPUTextureView.prototype.label;

/** @constructor */
function GPUExternalTexture() {}
/** @type {string} */
GPUExternalTexture.prototype.label;

/** @constructor */
function GPUSampler() {}
/** @type {string} */
GPUSampler.prototype.label;

/** @constructor */
function GPUBindGroupLayout() {}
/** @type {string} */
GPUBindGroupLayout.prototype.label;

/** @constructor */
function GPUBindGroup() {}
/** @type {string} */
GPUBindGroup.prototype.label;

/** @constructor */
function GPUPipelineLayout() {}
/** @type {string} */
GPUPipelineLayout.prototype.label;

/** @constructor */
function GPUShaderModule() {}
/** @type {string} */
GPUShaderModule.prototype.label;
/** @return {!Promise<!GPUCompilationInfo>} */
GPUShaderModule.prototype.getCompilationInfo = function(...args) {};

/** @constructor */
function GPUCompilationMessage() {}
/** @type {string} */
GPUCompilationMessage.prototype.message;
/** @type {string} */
GPUCompilationMessage.prototype.type;
/** @type {number} */
GPUCompilationMessage.prototype.lineNum;
/** @type {number} */
GPUCompilationMessage.prototype.linePos;
/** @type {number} */
GPUCompilationMessage.prototype.offset;
/** @type {number} */
GPUCompilationMessage.prototype.length;

/** @constructor */
function GPUCompilationInfo() {}
/** @type {!Array<!GPUCompilationMessage>} */
GPUCompilationInfo.prototype.messages;

/** @constructor */
function GPUPipelineError() {}
/** @type {string} */
GPUPipelineError.prototype.reason;

/** @constructor */
function GPUComputePipeline() {}
/** @type {string} */
GPUComputePipeline.prototype.label;
/** @return {!GPUBindGroupLayout} */
GPUComputePipeline.prototype.getBindGroupLayout = function(...args) {};

/** @constructor */
function GPURenderPipeline() {}
/** @type {string} */
GPURenderPipeline.prototype.label;
/** @return {!GPUBindGroupLayout} */
GPURenderPipeline.prototype.getBindGroupLayout = function(...args) {};

/** @constructor */
function GPUCommandBuffer() {}
/** @type {string} */
GPUCommandBuffer.prototype.label;

/** @constructor */
function GPUCommandEncoder() {}
/** @type {string} */
GPUCommandEncoder.prototype.label;
/** @return {undefined} */
GPUCommandEncoder.prototype.pushDebugGroup = function(...args) {};
/** @return {undefined} */
GPUCommandEncoder.prototype.popDebugGroup = function(...args) {};
/** @return {undefined} */
GPUCommandEncoder.prototype.insertDebugMarker = function(...args) {};
/** @return {!GPURenderPassEncoder} */
GPUCommandEncoder.prototype.beginRenderPass = function(...args) {};
/** @return {!GPUComputePassEncoder} */
GPUCommandEncoder.prototype.beginComputePass = function(...args) {};
/** @return {undefined} */
GPUCommandEncoder.prototype.copyBufferToBuffer = function(...args) {};
/** @return {undefined} */
GPUCommandEncoder.prototype.copyBufferToTexture = function(...args) {};
/** @return {undefined} */
GPUCommandEncoder.prototype.copyTextureToBuffer = function(...args) {};
/** @return {undefined} */
GPUCommandEncoder.prototype.copyTextureToTexture = function(...args) {};
/** @return {undefined} */
GPUCommandEncoder.prototype.clearBuffer = function(...args) {};
/** @return {undefined} */
GPUCommandEncoder.prototype.resolveQuerySet = function(...args) {};
/** @return {!GPUCommandBuffer} */
GPUCommandEncoder.prototype.finish = function(...args) {};

/** @constructor */
function GPUComputePassEncoder() {}
/** @type {string} */
GPUComputePassEncoder.prototype.label;
/** @return {undefined} */
GPUComputePassEncoder.prototype.pushDebugGroup = function(...args) {};
/** @return {undefined} */
GPUComputePassEncoder.prototype.popDebugGroup = function(...args) {};
/** @return {undefined} */
GPUComputePassEncoder.prototype.insertDebugMarker = function(...args) {};
/** @return {undefined} */
GPUComputePassEncoder.prototype.setBindGroup = function(...args) {};
/** @return {undefined} */
GPUComputePassEncoder.prototype.setBindGroup = function(...args) {};
/** @return {undefined} */
GPUComputePassEncoder.prototype.setPipeline = function(...args) {};
/** @return {undefined} */
GPUComputePassEncoder.prototype.dispatchWorkgroups = function(...args) {};
/** @return {undefined} */
GPUComputePassEncoder.prototype.dispatchWorkgroupsIndirect = function(...args) {};
/** @return {undefined} */
GPUComputePassEncoder.prototype.end = function(...args) {};

/** @constructor */
function GPURenderPassEncoder() {}
/** @type {string} */
GPURenderPassEncoder.prototype.label;
/** @return {undefined} */
GPURenderPassEncoder.prototype.pushDebugGroup = function(...args) {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.popDebugGroup = function(...args) {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.insertDebugMarker = function(...args) {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.setBindGroup = function(...args) {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.setBindGroup = function(...args) {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.setPipeline = function(...args) {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.setIndexBuffer = function(...args) {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.setVertexBuffer = function(...args) {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.draw = function(...args) {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.drawIndexed = function(...args) {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.drawIndirect = function(...args) {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.drawIndexedIndirect = function(...args) {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.setViewport = function(...args) {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.setScissorRect = function(...args) {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.setBlendConstant = function(...args) {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.setStencilReference = function(...args) {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.beginOcclusionQuery = function(...args) {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.endOcclusionQuery = function(...args) {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.executeBundles = function(...args) {};
/** @return {undefined} */
GPURenderPassEncoder.prototype.end = function(...args) {};

/** @constructor */
function GPURenderBundle() {}
/** @type {string} */
GPURenderBundle.prototype.label;

/** @constructor */
function GPURenderBundleEncoder() {}
/** @type {string} */
GPURenderBundleEncoder.prototype.label;
/** @return {undefined} */
GPURenderBundleEncoder.prototype.pushDebugGroup = function(...args) {};
/** @return {undefined} */
GPURenderBundleEncoder.prototype.popDebugGroup = function(...args) {};
/** @return {undefined} */
GPURenderBundleEncoder.prototype.insertDebugMarker = function(...args) {};
/** @return {undefined} */
GPURenderBundleEncoder.prototype.setBindGroup = function(...args) {};
/** @return {undefined} */
GPURenderBundleEncoder.prototype.setBindGroup = function(...args) {};
/** @return {undefined} */
GPURenderBundleEncoder.prototype.setPipeline = function(...args) {};
/** @return {undefined} */
GPURenderBundleEncoder.prototype.setIndexBuffer = function(...args) {};
/** @return {undefined} */
GPURenderBundleEncoder.prototype.setVertexBuffer = function(...args) {};
/** @return {undefined} */
GPURenderBundleEncoder.prototype.draw = function(...args) {};
/** @return {undefined} */
GPURenderBundleEncoder.prototype.drawIndexed = function(...args) {};
/** @return {undefined} */
GPURenderBundleEncoder.prototype.drawIndirect = function(...args) {};
/** @return {undefined} */
GPURenderBundleEncoder.prototype.drawIndexedIndirect = function(...args) {};
/** @return {!GPURenderBundle} */
GPURenderBundleEncoder.prototype.finish = function(...args) {};

/** @constructor */
function GPUQueue() {}
/** @type {string} */
GPUQueue.prototype.label;
/** @return {undefined} */
GPUQueue.prototype.submit = function(...args) {};
/** @return {!Promise<undefined>} */
GPUQueue.prototype.onSubmittedWorkDone = function(...args) {};
/** @return {undefined} */
GPUQueue.prototype.writeBuffer = function(...args) {};
/** @return {undefined} */
GPUQueue.prototype.writeTexture = function(...args) {};
/** @return {undefined} */
GPUQueue.prototype.copyExternalImageToTexture = function(...args) {};

/** @constructor */
function GPUQuerySet() {}
/** @type {string} */
GPUQuerySet.prototype.label;
/** @return {undefined} */
GPUQuerySet.prototype.destroy = function(...args) {};
/** @type {string} */
GPUQuerySet.prototype.type;
/** @type {number} */
GPUQuerySet.prototype.count;

/** @constructor */
function GPUCanvasContext() {}
/** @type {!HTMLCanvasElement|!OffscreenCanvas} */
GPUCanvasContext.prototype.canvas;
/** @return {undefined} */
GPUCanvasContext.prototype.configure = function(...args) {};
/** @return {undefined} */
GPUCanvasContext.prototype.unconfigure = function(...args) {};
/** @return {!GPUTexture} */
GPUCanvasContext.prototype.getCurrentTexture = function(...args) {};

/** @constructor */
function GPUDeviceLostInfo() {}
/** @type {string} */
GPUDeviceLostInfo.prototype.reason;
/** @type {string} */
GPUDeviceLostInfo.prototype.message;

/** @constructor */
function GPUError() {}
/** @type {string} */
GPUError.prototype.message;

/** @constructor */
function GPUValidationError() {}

/** @constructor */
function GPUOutOfMemoryError() {}

/** @constructor */
function GPUInternalError() {}

/** @constructor */
function GPUUncapturedErrorEvent() {}
/** @type {!GPUError} */
GPUUncapturedErrorEvent.prototype.error;
