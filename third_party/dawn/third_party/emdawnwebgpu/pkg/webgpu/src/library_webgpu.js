/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

//
// This file and webgpu.cpp together implement <webgpu/webgpu.h>.
//

{{{
  if (USE_WEBGPU || !__HAVE_EMDAWNWEBGPU_STRUCT_INFO || !__HAVE_EMDAWNWEBGPU_ENUM_TABLES || !__HAVE_EMDAWNWEBGPU_SIG_INFO) {
    throw new Error("To use Dawn's library_webgpu.js, disable -sUSE_WEBGPU and first include Dawn's library_webgpu_struct_info.js and library_webgpu_enum_tables.js (before library_webgpu.js)");
  }

  // Helper functions for code generation
  globalThis.gpu = {
    convertSentinelToUndefined: function(name, type) {
      // The sentinel value SIZE_MAX is passed as a "p" (pointer) arg, so it comes through as
      // either `0xFFFFFFFF` or `-1` depending on whether `CAN_ADDRESS_2GB` is enabled.
      if (type === '*') type = MEMORY64 ? 'i53' : CAN_ADDRESS_2GB ? 'u32' : 'i32';

      if (type === 'u32') {
        return `if (${name} == 0xFFFFFFFF) ${name} = undefined;`;
      } else if (type === 'i32' || type === 'i53') {
        return `if (${name} == -1) ${name} = undefined;`;
      } else {
        throw new Error('type not supported: ' + type);
      }
    },

    NULLPTR: MEMORY64 ? '0n' : '0',
    kOffsetOfNextInChainMember: 0,
    passAsI64: value => WASM_BIGINT ? `BigInt(${value})` : value,
    passAsPointer: value => MEMORY64 ? `BigInt(${value})` : value,
    convertToPassAsPointer: variable => MEMORY64 ? `${variable} = BigInt(${variable});` : '',

    // Helpers used to convert from the default JS interpretation of signed ints to unsigned ints.
    // Note that we use |convertToU31| for values we assume should always be small so that we only
    // assert it in debug mode. We use |convertToU32| for values that may be unsigned values that
    // can validly be larger than 2^31 such that the signed bit may be flipped.
    convertToU31: function(variable) {
      if (!ASSERTIONS) return '';
      return `assert(${variable} >= 0);`;
    },
    convertToU32: function(variable) {
      return `${variable} >>>= 0;`;
    },

    // Provide very limited support for mismatches between the compile options for webgpu.cpp and
    // the link options for the program, in a specific unknown case where negative pointers get
    // passed from Wasm to JS. TODO(b/422847728): We shouldn't need this. Try to remove it.
    ensurePointerUnsigned: function(variable) {
      if (MEMORY64) return '';
      return `${variable} >>>= 0`;
    },

    // Helpers for getting specific types out of memory.
    // Numeric types should just use `makeGetValue`, notably:
    // - '*' for pointers and sizes
    // - 'i53' for both uint64_t and int64_t where values >= 2**53 don't matter
    //   (aside from UINT64_MAX which maps to -1). We don't bother to use 'u53'
    //   because it's slightly extra code and it only does anything >= 2**63.
    makeGetBool: function(struct, offset) {
      return `!!(${makeGetValue(struct, offset, 'u32')})`;
    },
    makeGetEnum: function(struct, offset, tableName) {
      // TODO(crbug.com/436751438): Check that the enum value isn't unknown.
      return `WebGPU.${tableName}[${makeGetValue(struct, offset, 'i32')}]`;
    },
    makeCheck: function(str) {
      if (!ASSERTIONS) return '';
      return `assert(${str});`;
    },
    makeCheckDescriptor: function(descriptor) {
      // Assert descriptor is non-null, then that its nextInChain is null.
      // For descriptors that aren't the first in the chain (e.g
      // ShaderSourceSPIRV), there is no .nextInChain pointer, but
      // instead a ChainedStruct object: .chain. So we need to check if
      // .chain.nextInChain is null. As long as nextInChain and chain are always
      // the first member in the struct, descriptor.nextInChain and
      // descriptor.chain.nextInChain should have the same offset (0) to the
      // descriptor pointer and we can check it to be null.
      return this.makeCheck(descriptor) + this.makeCheck(makeGetValue(descriptor, gpu.kOffsetOfNextInChainMember, '*') + ' === 0');
    },
    makeImportJsObject: function(object) {
      return `
        importJs${object}__deps: ['emwgpuCreate${object}'],
        importJs${object}: (obj, parentPtr = 0) => {
          var ptr = _emwgpuCreate${object}(parentPtr);
          WebGPU.Internals.jsObjects[ptr] = obj;
          return ptr;
        },
      `
    },

    // Compile-time table for enum integer values used with templating.
    // Must be in sync with webgpu.h.
    // TODO: Generate this to keep it in sync with webgpu.h
    COPY_STRIDE_UNDEFINED: 0xFFFFFFFF,
    LIMIT_U32_UNDEFINED: 0xFFFFFFFF,
    MIP_LEVEL_COUNT_UNDEFINED: 0xFFFFFFFF,
    ARRAY_LAYER_COUNT_UNDEFINED: 0xFFFFFFFF,
    ...WEBGPU_ENUM_CONSTANT_TABLES,
  };
  null;
}}}

var LibraryWebGPU = {
  $WebGPU__deps: [],
  $WebGPU: {
    // Note that external users should not depend on any of the internal
    // implementation details in this sub-object, as they are subject to
    // change to support new features or optimizations. Instead, external
    // users should rely only on the public APIs.
    Internals: {
      // Object management is consolidated into a single table that doesn't
      // care about object type, and is keyed on the pointer address.
      jsObjects: [],
      jsObjectInsert: (ptr, jsObject) => {
        {{{ gpu.ensurePointerUnsigned('ptr') }}}
        WebGPU.Internals.jsObjects[ptr] = jsObject;
      },

      // Buffer unmapping callbacks are stored in a separate table to keep
      // the jsObject table simple.
      bufferOnUnmaps: [],

      // Future to promise management. Note that all FutureIDs (uint64_t) are
      // passed either as a low and high value or by pointer because they need
      // to be passed back and forth between JS and C++, and JS is currently
      // unable to pass a value to a C++ function as a uint64_t. This might be
      // possible with -sWASM_BIGINT, but I was unable to get that to work
      // properly at the time of writing.
      futures: [],
      futureInsert: (futureId, promise) => {
#if ASYNCIFY
        WebGPU.Internals.futures[futureId] =
          new Promise((resolve) => promise.finally(() => resolve(futureId)));
#endif
      },
    },

    // Public utility functions useful for translating between WASM/JS. Most of
    // the import utilities are generated, with some exceptions that are
    // explicitly implemented because they have some slight differences. Note
    // that all import functions take the expected GPUObject JS types as the
    // first argument and an optional second argument that is WGPUObject C type
    // (i.e. pointer), that should extend EventSource from webgpu.cpp. The
    // second argument is a "parent" object that's needed in order to handle
    // WGPUFutures when using in non-AllowSpontaneous mode. For most objects,
    // a WGPUDevice would suffice as a parent. For a WGPUDevice, either a
    // WGPUAdapter or WGPUInstance would be valid. Also note that imported
    // objects are not cleaned up as objects created natively via the APIs
    // because importing is not a "move" into the API, rather just a "copy".
    getJsObject: (ptr) => {
      if (!ptr) return undefined;
      {{{ gpu.ensurePointerUnsigned('ptr') }}}
#if ASSERTIONS
      assert(ptr in WebGPU.Internals.jsObjects);
#endif
      return WebGPU.Internals.jsObjects[ptr];
    },
    {{{ gpu.makeImportJsObject('Adapter') }}}
    {{{ gpu.makeImportJsObject('BindGroup') }}}
    {{{ gpu.makeImportJsObject('BindGroupLayout') }}}
    importJsBuffer__deps: ['emwgpuImportBuffer'],
    importJsBuffer: (buffer, parentPtr = 0) => {
      // At the moment, we do not allow importing pending buffers.
      assert(buffer.mapState === "unmapped");
      var bufferPtr = _emwgpuImportBuffer(parentPtr);
      WebGPU.Internals.jsObjectInsert(bufferPtr, buffer);
      return bufferPtr;
    },
    {{{ gpu.makeImportJsObject('CommandBuffer') }}}
    {{{ gpu.makeImportJsObject('CommandEncoder') }}}
    {{{ gpu.makeImportJsObject('ComputePassEncoder') }}}
    {{{ gpu.makeImportJsObject('ComputePipeline') }}}
    importJsDevice__deps: ['emwgpuCreateDevice', 'emwgpuCreateQueue'],
    importJsDevice: (device, parentPtr = 0) => {
      var queuePtr = _emwgpuCreateQueue(parentPtr);
      var devicePtr = _emwgpuCreateDevice(parentPtr, queuePtr);
      WebGPU.Internals.jsObjectInsert(queuePtr, device.queue);
      WebGPU.Internals.jsObjectInsert(devicePtr, device);
      return devicePtr;
    },
    {{{ gpu.makeImportJsObject('ExternalTexture') }}}
    {{{ gpu.makeImportJsObject('PipelineLayout') }}}
    {{{ gpu.makeImportJsObject('QuerySet') }}}
    {{{ gpu.makeImportJsObject('Queue') }}}
    {{{ gpu.makeImportJsObject('RenderBundle') }}}
    {{{ gpu.makeImportJsObject('RenderBundleEncoder') }}}
    {{{ gpu.makeImportJsObject('RenderPassEncoder') }}}
    {{{ gpu.makeImportJsObject('RenderPipeline') }}}
    {{{ gpu.makeImportJsObject('Sampler') }}}
    {{{ gpu.makeImportJsObject('ShaderModule') }}}
    {{{ gpu.makeImportJsObject('Surface') }}}
    {{{ gpu.makeImportJsObject('Texture') }}}
    {{{ gpu.makeImportJsObject('TextureView') }}}

    // TODO(crbug.com/42241415): Remove this after verifying that it's not used and/or updating users.
    errorCallback__deps: ['$stackSave', '$stackRestore', '$stringToUTF8OnStack'],
    errorCallback: (callback, type, message, userdata) => {
      var sp = stackSave();
      var messagePtr = stringToUTF8OnStack(message);
      {{{ makeDynCall('vipp', 'callback') }}}(type, {{{ gpu.passAsPointer('messagePtr') }}}, userdata);
      stackRestore(sp);
    },

    iterateExtensions: (root, handlers) => {
      {{{ gpu.makeCheck('root') }}}
      for (var ptr = {{{ makeGetValue('root', gpu.kOffsetOfNextInChainMember, '*') }}}; ptr;
               ptr = {{{ makeGetValue('ptr', C_STRUCTS.WGPUChainedStruct.next, '*') }}}) {
        var sType = {{{ makeGetValue('ptr', C_STRUCTS.WGPUChainedStruct.sType, 'i32') }}};
        // This will crash if there's no handler indicating either a bogus
        // sType, or one we haven't implemented yet.
        var handler = handlers[sType](ptr);
      }
    },

    setStringView: (ptr, data, length) => {
      {{{ makeSetValue('ptr', C_STRUCTS.WGPUStringView.data, 'data', '*') }}};
      {{{ makeSetValue('ptr', C_STRUCTS.WGPUStringView.length, 'length', '*') }}};
    },

    makeStringFromStringView__deps: ['$UTF8ToString'],
    makeStringFromStringView: (stringViewPtr) => {
      var ptr = {{{ makeGetValue('stringViewPtr', C_STRUCTS.WGPUStringView.data, '*') }}};
      var length = {{{ makeGetValue('stringViewPtr', C_STRUCTS.WGPUStringView.length, '*') }}};
      // UTF8ToString stops at the first null terminator character in the
      // string regardless of the length.
      return UTF8ToString(ptr, length);
    },
    makeStringFromOptionalStringView__deps: ['$UTF8ToString'],
    makeStringFromOptionalStringView: (stringViewPtr) => {
      var ptr = {{{ makeGetValue('stringViewPtr', C_STRUCTS.WGPUStringView.data, '*') }}};
      var length = {{{ makeGetValue('stringViewPtr', C_STRUCTS.WGPUStringView.length, '*') }}};
      // If we don't have a valid string pointer, just return undefined when
      // optional.
      if (!ptr) {
        if (length === 0) {
          return "";
        }
        return undefined;
      }
      // UTF8ToString stops at the first null terminator character in the
      // string regardless of the length.
      return UTF8ToString(ptr, length);
    },

    makeColor: (ptr) => {
      return {
        "r": {{{ makeGetValue('ptr', 0, 'double') }}},
        "g": {{{ makeGetValue('ptr', 8, 'double') }}},
        "b": {{{ makeGetValue('ptr', 16, 'double') }}},
        "a": {{{ makeGetValue('ptr', 24, 'double') }}},
      };
    },

    makeExtent3D: (ptr) => {
      return {
        "width": {{{ makeGetValue('ptr', C_STRUCTS.WGPUExtent3D.width, 'u32') }}},
        "height": {{{ makeGetValue('ptr', C_STRUCTS.WGPUExtent3D.height, 'u32') }}},
        "depthOrArrayLayers": {{{ makeGetValue('ptr', C_STRUCTS.WGPUExtent3D.depthOrArrayLayers, 'u32') }}},
      };
    },

    makeOrigin3D: (ptr) => {
      return {
        "x": {{{ makeGetValue('ptr', C_STRUCTS.WGPUOrigin3D.x, 'u32') }}},
        "y": {{{ makeGetValue('ptr', C_STRUCTS.WGPUOrigin3D.y, 'u32') }}},
        "z": {{{ makeGetValue('ptr', C_STRUCTS.WGPUOrigin3D.z, 'u32') }}},
      };
    },

    makeTexelCopyTextureInfo: (ptr) => {
      {{{ gpu.makeCheck('ptr') }}}
      return {
        "texture": WebGPU.getJsObject(
          {{{ makeGetValue('ptr', C_STRUCTS.WGPUTexelCopyTextureInfo.texture, '*') }}}),
        "mipLevel": {{{ makeGetValue('ptr', C_STRUCTS.WGPUTexelCopyTextureInfo.mipLevel, 'u32') }}},
        "origin": WebGPU.makeOrigin3D(ptr + {{{ C_STRUCTS.WGPUTexelCopyTextureInfo.origin }}}),
        "aspect": {{{ gpu.makeGetEnum('ptr', C_STRUCTS.WGPUTexelCopyTextureInfo.aspect, 'TextureAspect') }}},
      };
    },

    makeTexelCopyBufferLayout: (ptr) => {
      var bytesPerRow = {{{ makeGetValue('ptr', C_STRUCTS.WGPUTexelCopyBufferLayout.bytesPerRow, 'u32') }}};
      var rowsPerImage = {{{ makeGetValue('ptr', C_STRUCTS.WGPUTexelCopyBufferLayout.rowsPerImage, 'u32') }}};
      return {
        "offset": {{{ makeGetValue('ptr', C_STRUCTS.WGPUTexelCopyBufferLayout.offset, 'i53') }}},
        "bytesPerRow": bytesPerRow === {{{ gpu.COPY_STRIDE_UNDEFINED }}} ? undefined : bytesPerRow,
        "rowsPerImage": rowsPerImage === {{{ gpu.COPY_STRIDE_UNDEFINED }}} ? undefined : rowsPerImage,
      };
    },

    makeTexelCopyBufferInfo: (ptr) => {
      {{{ gpu.makeCheck('ptr') }}}
      var layoutPtr = ptr + {{{ C_STRUCTS.WGPUTexelCopyBufferInfo.layout }}};
      var bufferCopyView = WebGPU.makeTexelCopyBufferLayout(layoutPtr);
      bufferCopyView["buffer"] = WebGPU.getJsObject(
        {{{ makeGetValue('ptr', C_STRUCTS.WGPUTexelCopyBufferInfo.buffer, '*') }}});
      return bufferCopyView;
    },

    makePassTimestampWrites: (ptr) => {
      if (ptr === 0) return undefined;
      return {
        "querySet": WebGPU.getJsObject(
          {{{ makeGetValue('ptr', C_STRUCTS.WGPUPassTimestampWrites.querySet, '*') }}}),
        "beginningOfPassWriteIndex": {{{ makeGetValue('ptr', C_STRUCTS.WGPUPassTimestampWrites.beginningOfPassWriteIndex, 'u32') }}},
        "endOfPassWriteIndex": {{{ makeGetValue('ptr', C_STRUCTS.WGPUPassTimestampWrites.endOfPassWriteIndex, 'u32') }}},
      };
    },

    makePipelineConstants: (constantCount, constantsPtr) => {
      if (!constantCount) return;
      var constants = {};
      for (var i = 0; i < constantCount; ++i) {
        var entryPtr = constantsPtr + {{{ C_STRUCTS.WGPUConstantEntry.__size__ }}} * i;
        var key = WebGPU.makeStringFromStringView(entryPtr + {{{ C_STRUCTS.WGPUConstantEntry.key }}});
        constants[key] = {{{ makeGetValue('entryPtr', C_STRUCTS.WGPUConstantEntry.value, 'double') }}};
      }
      return constants;
    },

    makePipelineLayout: (layoutPtr) => {
      if (!layoutPtr) return 'auto';
      return WebGPU.getJsObject(layoutPtr);
    },

    makeComputeState: (ptr) => {
      if (!ptr) return undefined;
      {{{ gpu.makeCheckDescriptor('ptr') }}}
      var desc = {
        "module": WebGPU.getJsObject(
          {{{ makeGetValue('ptr', C_STRUCTS.WGPUComputeState.module, '*') }}}),
        "constants": WebGPU.makePipelineConstants(
          {{{ makeGetValue('ptr', C_STRUCTS.WGPUComputeState.constantCount, '*') }}},
          {{{ makeGetValue('ptr', C_STRUCTS.WGPUComputeState.constants, '*') }}}),
        "entryPoint": WebGPU.makeStringFromOptionalStringView(
          ptr + {{{ C_STRUCTS.WGPUComputeState.entryPoint }}}),
      };
      return desc;
    },

    makeComputePipelineDesc: (descriptor) => {
      {{{ gpu.makeCheckDescriptor('descriptor') }}}

      var desc = {
        "label": WebGPU.makeStringFromOptionalStringView(
          descriptor + {{{ C_STRUCTS.WGPUComputePipelineDescriptor.label }}}),
        "layout": WebGPU.makePipelineLayout(
          {{{ makeGetValue('descriptor', C_STRUCTS.WGPUComputePipelineDescriptor.layout, '*') }}}),
        "compute": WebGPU.makeComputeState(
          descriptor + {{{ C_STRUCTS.WGPUComputePipelineDescriptor.compute }}}),
      };
      return desc;
    },

    makeRenderPipelineDesc: (descriptor) => {
      {{{ gpu.makeCheckDescriptor('descriptor') }}}

      function makePrimitiveState(psPtr) {
        if (!psPtr) return undefined;
        {{{ gpu.makeCheckDescriptor('psPtr') }}}
        return {
          "topology": {{{ gpu.makeGetEnum('psPtr', C_STRUCTS.WGPUPrimitiveState.topology, 'PrimitiveTopology') }}},
          "stripIndexFormat": {{{ gpu.makeGetEnum('psPtr', C_STRUCTS.WGPUPrimitiveState.stripIndexFormat, 'IndexFormat') }}},
          "frontFace": {{{ gpu.makeGetEnum('psPtr', C_STRUCTS.WGPUPrimitiveState.frontFace, 'FrontFace') }}},
          "cullMode": {{{ gpu.makeGetEnum('psPtr', C_STRUCTS.WGPUPrimitiveState.cullMode, 'CullMode') }}},
          "unclippedDepth": {{{ gpu.makeGetBool('psPtr', C_STRUCTS.WGPUPrimitiveState.unclippedDepth) }}},
        };
      }

      function makeBlendComponent(bdPtr) {
        if (!bdPtr) return undefined;
        return {
          "operation": {{{ gpu.makeGetEnum('bdPtr', C_STRUCTS.WGPUBlendComponent.operation, 'BlendOperation') }}},
          "srcFactor": {{{ gpu.makeGetEnum('bdPtr', C_STRUCTS.WGPUBlendComponent.srcFactor, 'BlendFactor') }}},
          "dstFactor": {{{ gpu.makeGetEnum('bdPtr', C_STRUCTS.WGPUBlendComponent.dstFactor, 'BlendFactor') }}},
        };
      }

      function makeBlendState(bsPtr) {
        if (!bsPtr) return undefined;
        return {
          "alpha": makeBlendComponent(bsPtr + {{{ C_STRUCTS.WGPUBlendState.alpha }}}),
          "color": makeBlendComponent(bsPtr + {{{ C_STRUCTS.WGPUBlendState.color }}}),
        };
      }

      function makeColorState(csPtr) {
        {{{ gpu.makeCheckDescriptor('csPtr') }}}
        var format = {{{ gpu.makeGetEnum('csPtr', C_STRUCTS.WGPUColorTargetState.format, 'TextureFormat') }}};
        return format ? {
          "format": format,
          "blend": makeBlendState({{{ makeGetValue('csPtr', C_STRUCTS.WGPUColorTargetState.blend, '*') }}}),
          "writeMask": {{{ makeGetValue('csPtr', C_STRUCTS.WGPUColorTargetState.writeMask, 'u32') }}},
        } : undefined;
      }

      function makeColorStates(count, csArrayPtr) {
        var states = [];
        for (var i = 0; i < count; ++i) {
          states.push(makeColorState(csArrayPtr + {{{ C_STRUCTS.WGPUColorTargetState.__size__ }}} * i));
        }
        return states;
      }

      function makeStencilStateFace(ssfPtr) {
        {{{ gpu.makeCheck('ssfPtr') }}}
        return {
          "compare": {{{ gpu.makeGetEnum('ssfPtr', C_STRUCTS.WGPUStencilFaceState.compare, 'CompareFunction') }}},
          "failOp": {{{ gpu.makeGetEnum('ssfPtr', C_STRUCTS.WGPUStencilFaceState.failOp, 'StencilOperation') }}},
          "depthFailOp": {{{ gpu.makeGetEnum('ssfPtr', C_STRUCTS.WGPUStencilFaceState.depthFailOp, 'StencilOperation') }}},
          "passOp": {{{ gpu.makeGetEnum('ssfPtr', C_STRUCTS.WGPUStencilFaceState.passOp, 'StencilOperation') }}},
        };
      }

      function makeDepthStencilState(dssPtr) {
        if (!dssPtr) return undefined;

        {{{ gpu.makeCheck('dssPtr') }}}
        return {
          "format": {{{ gpu.makeGetEnum('dssPtr', C_STRUCTS.WGPUDepthStencilState.format, 'TextureFormat') }}},
          "depthWriteEnabled": {{{ gpu.makeGetBool('dssPtr', C_STRUCTS.WGPUDepthStencilState.depthWriteEnabled) }}},
          "depthCompare": {{{ gpu.makeGetEnum('dssPtr', C_STRUCTS.WGPUDepthStencilState.depthCompare, 'CompareFunction') }}},
          "stencilFront": makeStencilStateFace(dssPtr + {{{ C_STRUCTS.WGPUDepthStencilState.stencilFront }}}),
          "stencilBack": makeStencilStateFace(dssPtr + {{{ C_STRUCTS.WGPUDepthStencilState.stencilBack }}}),
          "stencilReadMask": {{{ makeGetValue('dssPtr', C_STRUCTS.WGPUDepthStencilState.stencilReadMask, 'u32') }}},
          "stencilWriteMask": {{{ makeGetValue('dssPtr', C_STRUCTS.WGPUDepthStencilState.stencilWriteMask, 'u32') }}},
          "depthBias": {{{ makeGetValue('dssPtr', C_STRUCTS.WGPUDepthStencilState.depthBias, 'i32') }}},
          "depthBiasSlopeScale": {{{ makeGetValue('dssPtr', C_STRUCTS.WGPUDepthStencilState.depthBiasSlopeScale, 'float') }}},
          "depthBiasClamp": {{{ makeGetValue('dssPtr', C_STRUCTS.WGPUDepthStencilState.depthBiasClamp, 'float') }}},
        };
      }

      function makeVertexAttribute(vaPtr) {
        {{{ gpu.makeCheck('vaPtr') }}}
        return {
          "format": {{{ gpu.makeGetEnum('vaPtr', C_STRUCTS.WGPUVertexAttribute.format, 'VertexFormat') }}},
          "offset": {{{ makeGetValue('vaPtr', C_STRUCTS.WGPUVertexAttribute.offset, 'i53') }}},
          "shaderLocation": {{{ makeGetValue('vaPtr', C_STRUCTS.WGPUVertexAttribute.shaderLocation, 'u32') }}},
        };
      }

      function makeVertexAttributes(count, vaArrayPtr) {
        var vas = [];
        for (var i = 0; i < count; ++i) {
          vas.push(makeVertexAttribute(vaArrayPtr + i * {{{ C_STRUCTS.WGPUVertexAttribute.__size__ }}}));
        }
        return vas;
      }

      function makeVertexBuffer(vbPtr) {
        if (!vbPtr) return undefined;
        var stepMode = {{{ gpu.makeGetEnum('vbPtr',C_STRUCTS.WGPUVertexBufferLayout.stepMode, 'VertexStepMode') }}};
        var attributeCount = {{{ makeGetValue('vbPtr', C_STRUCTS.WGPUVertexBufferLayout.attributeCount, '*') }}};
        if (!stepMode && !attributeCount) {
          return null;
        }
        return {
          "arrayStride": {{{ makeGetValue('vbPtr', C_STRUCTS.WGPUVertexBufferLayout.arrayStride, 'i53') }}},
          "stepMode": stepMode,
          "attributes": makeVertexAttributes(
            attributeCount,
            {{{ makeGetValue('vbPtr', C_STRUCTS.WGPUVertexBufferLayout.attributes, '*') }}}),
        };
      }

      function makeVertexBuffers(count, vbArrayPtr) {
        if (!count) return undefined;

        var vbs = [];
        for (var i = 0; i < count; ++i) {
          vbs.push(makeVertexBuffer(vbArrayPtr + i * {{{ C_STRUCTS.WGPUVertexBufferLayout.__size__ }}}));
        }
        return vbs;
      }

      function makeVertexState(viPtr) {
        if (!viPtr) return undefined;
        {{{ gpu.makeCheckDescriptor('viPtr') }}}
        var desc = {
          "module": WebGPU.getJsObject(
            {{{ makeGetValue('viPtr', C_STRUCTS.WGPUVertexState.module, '*') }}}),
          "constants": WebGPU.makePipelineConstants(
            {{{ makeGetValue('viPtr', C_STRUCTS.WGPUVertexState.constantCount, '*') }}},
            {{{ makeGetValue('viPtr', C_STRUCTS.WGPUVertexState.constants, '*') }}}),
          "buffers": makeVertexBuffers(
            {{{ makeGetValue('viPtr', C_STRUCTS.WGPUVertexState.bufferCount, '*') }}},
            {{{ makeGetValue('viPtr', C_STRUCTS.WGPUVertexState.buffers, '*') }}}),
          "entryPoint": WebGPU.makeStringFromOptionalStringView(
            viPtr + {{{ C_STRUCTS.WGPUVertexState.entryPoint }}}),
          };
        return desc;
      }

      function makeMultisampleState(msPtr) {
        if (!msPtr) return undefined;
        {{{ gpu.makeCheckDescriptor('msPtr') }}}
        return {
          "count": {{{ makeGetValue('msPtr', C_STRUCTS.WGPUMultisampleState.count, 'u32') }}},
          "mask": {{{ makeGetValue('msPtr', C_STRUCTS.WGPUMultisampleState.mask, 'u32') }}},
          "alphaToCoverageEnabled": {{{ gpu.makeGetBool('msPtr', C_STRUCTS.WGPUMultisampleState.alphaToCoverageEnabled) }}},
        };
      }

      function makeFragmentState(fsPtr) {
        if (!fsPtr) return undefined;
        {{{ gpu.makeCheckDescriptor('fsPtr') }}}
        var desc = {
          "module": WebGPU.getJsObject(
            {{{ makeGetValue('fsPtr', C_STRUCTS.WGPUFragmentState.module, '*') }}}),
          "constants": WebGPU.makePipelineConstants(
            {{{ makeGetValue('fsPtr', C_STRUCTS.WGPUFragmentState.constantCount, '*') }}},
            {{{ makeGetValue('fsPtr', C_STRUCTS.WGPUFragmentState.constants, '*') }}}),
          "targets": makeColorStates(
            {{{ makeGetValue('fsPtr', C_STRUCTS.WGPUFragmentState.targetCount, '*') }}},
            {{{ makeGetValue('fsPtr', C_STRUCTS.WGPUFragmentState.targets, '*') }}}),
          "entryPoint": WebGPU.makeStringFromOptionalStringView(
            fsPtr + {{{ C_STRUCTS.WGPUFragmentState.entryPoint }}}),
          };
        return desc;
      }

      var desc = {
        "label": WebGPU.makeStringFromOptionalStringView(
          descriptor + {{{ C_STRUCTS.WGPURenderPipelineDescriptor.label }}}),
        "layout": WebGPU.makePipelineLayout(
          {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPipelineDescriptor.layout, '*') }}}),
        "vertex": makeVertexState(
          descriptor + {{{ C_STRUCTS.WGPURenderPipelineDescriptor.vertex }}}),
        "primitive": makePrimitiveState(
          descriptor + {{{ C_STRUCTS.WGPURenderPipelineDescriptor.primitive }}}),
        "depthStencil": makeDepthStencilState(
          {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPipelineDescriptor.depthStencil, '*') }}}),
        "multisample": makeMultisampleState(
          descriptor + {{{ C_STRUCTS.WGPURenderPipelineDescriptor.multisample }}}),
        "fragment": makeFragmentState(
          {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPipelineDescriptor.fragment, '*') }}}),
      };
      return desc;
    },

    fillLimitStruct__deps: ['$writeI53ToI64'],
    fillLimitStruct: (limits, limitsOutPtr) => {
      {{{ gpu.makeCheck('limitsOutPtr') }}}
      var nextInChainPtr = {{{ makeGetValue('limitsOutPtr', C_STRUCTS.WGPULimits.nextInChain, '*') }}};

      function setLimitValueU32(name, basePtr, limitOffset, fallbackValue = 0) {
        var limitValue = limits[name] ?? fallbackValue;
        {{{ makeSetValue('basePtr', 'limitOffset', 'limitValue', 'u32') }}};
      }
      function setLimitValueU64(name, basePtr, limitOffset, fallbackValue = 0) {
        var limitValue = limits[name] ?? fallbackValue;
        // Limits are integer-valued JS `Number`s, so they fit in 'i53'.
        {{{ makeSetValue('basePtr', 'limitOffset', 'limitValue', 'i53') }}};
      }

      setLimitValueU32('maxTextureDimension1D',                     limitsOutPtr, {{{ C_STRUCTS.WGPULimits.maxTextureDimension1D }}});
      setLimitValueU32('maxTextureDimension2D',                     limitsOutPtr, {{{ C_STRUCTS.WGPULimits.maxTextureDimension2D }}});
      setLimitValueU32('maxTextureDimension3D',                     limitsOutPtr, {{{ C_STRUCTS.WGPULimits.maxTextureDimension3D }}});
      setLimitValueU32('maxTextureArrayLayers',                     limitsOutPtr, {{{ C_STRUCTS.WGPULimits.maxTextureArrayLayers }}});
      setLimitValueU32('maxBindGroups',                             limitsOutPtr, {{{ C_STRUCTS.WGPULimits.maxBindGroups }}});
      setLimitValueU32('maxBindGroupsPlusVertexBuffers',            limitsOutPtr, {{{ C_STRUCTS.WGPULimits.maxBindGroupsPlusVertexBuffers }}});
      setLimitValueU32('maxBindingsPerBindGroup',                   limitsOutPtr, {{{ C_STRUCTS.WGPULimits.maxBindingsPerBindGroup }}});
      setLimitValueU32('maxDynamicUniformBuffersPerPipelineLayout', limitsOutPtr, {{{ C_STRUCTS.WGPULimits.maxDynamicUniformBuffersPerPipelineLayout }}});
      setLimitValueU32('maxDynamicStorageBuffersPerPipelineLayout', limitsOutPtr, {{{ C_STRUCTS.WGPULimits.maxDynamicStorageBuffersPerPipelineLayout }}});
      setLimitValueU32('maxSampledTexturesPerShaderStage',          limitsOutPtr, {{{ C_STRUCTS.WGPULimits.maxSampledTexturesPerShaderStage }}});
      setLimitValueU32('maxSamplersPerShaderStage',                 limitsOutPtr, {{{ C_STRUCTS.WGPULimits.maxSamplersPerShaderStage }}});
      setLimitValueU32('maxStorageBuffersPerShaderStage',           limitsOutPtr, {{{ C_STRUCTS.WGPULimits.maxStorageBuffersPerShaderStage }}});
      setLimitValueU32('maxStorageTexturesPerShaderStage',          limitsOutPtr, {{{ C_STRUCTS.WGPULimits.maxStorageTexturesPerShaderStage }}});
      setLimitValueU32('maxUniformBuffersPerShaderStage',           limitsOutPtr, {{{ C_STRUCTS.WGPULimits.maxUniformBuffersPerShaderStage }}});
      setLimitValueU32('minUniformBufferOffsetAlignment',           limitsOutPtr, {{{ C_STRUCTS.WGPULimits.minUniformBufferOffsetAlignment }}});
      setLimitValueU32('minStorageBufferOffsetAlignment',           limitsOutPtr, {{{ C_STRUCTS.WGPULimits.minStorageBufferOffsetAlignment }}});
      setLimitValueU64('maxUniformBufferBindingSize',               limitsOutPtr, {{{ C_STRUCTS.WGPULimits.maxUniformBufferBindingSize }}});
      setLimitValueU64('maxStorageBufferBindingSize',               limitsOutPtr, {{{ C_STRUCTS.WGPULimits.maxStorageBufferBindingSize }}});
      setLimitValueU32('maxVertexBuffers',                          limitsOutPtr, {{{ C_STRUCTS.WGPULimits.maxVertexBuffers }}});
      setLimitValueU64('maxBufferSize',                             limitsOutPtr, {{{ C_STRUCTS.WGPULimits.maxBufferSize }}});
      setLimitValueU32('maxVertexAttributes',                       limitsOutPtr, {{{ C_STRUCTS.WGPULimits.maxVertexAttributes }}});
      setLimitValueU32('maxVertexBufferArrayStride',                limitsOutPtr, {{{ C_STRUCTS.WGPULimits.maxVertexBufferArrayStride }}});
      setLimitValueU32('maxInterStageShaderVariables',              limitsOutPtr, {{{ C_STRUCTS.WGPULimits.maxInterStageShaderVariables }}});
      setLimitValueU32('maxColorAttachments',                       limitsOutPtr, {{{ C_STRUCTS.WGPULimits.maxColorAttachments }}});
      setLimitValueU32('maxColorAttachmentBytesPerSample',          limitsOutPtr, {{{ C_STRUCTS.WGPULimits.maxColorAttachmentBytesPerSample }}});
      setLimitValueU32('maxComputeWorkgroupStorageSize',            limitsOutPtr, {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupStorageSize }}});
      setLimitValueU32('maxComputeInvocationsPerWorkgroup',         limitsOutPtr, {{{ C_STRUCTS.WGPULimits.maxComputeInvocationsPerWorkgroup }}});
      setLimitValueU32('maxComputeWorkgroupSizeX',                  limitsOutPtr, {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupSizeX }}});
      setLimitValueU32('maxComputeWorkgroupSizeY',                  limitsOutPtr, {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupSizeY }}});
      setLimitValueU32('maxComputeWorkgroupSizeZ',                  limitsOutPtr, {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupSizeZ }}});
      setLimitValueU32('maxComputeWorkgroupsPerDimension',          limitsOutPtr, {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupsPerDimension }}});
      // Note this limit is new and won't be present in all browsers for a while. Fall back to 0.
      setLimitValueU32('maxImmediateSize',                          limitsOutPtr, {{{ C_STRUCTS.WGPULimits.maxImmediateSize }}});

      if (nextInChainPtr !== 0) {
        var sType = {{{ makeGetValue('nextInChainPtr', C_STRUCTS.WGPUChainedStruct.sType, 'i32') }}};
  #if ASSERTIONS
        assert(sType === {{{ gpu.SType.CompatibilityModeLimits }}});
        assert(0 === {{{ makeGetValue('nextInChainPtr', gpu.kOffsetOfNextInChainMember, '*') }}});
  #endif
        var compatibilityModeLimitsPtr = nextInChainPtr;
        {{{ gpu.makeCheckDescriptor('compatibilityModeLimitsPtr') }}}

        // Note these limits are new and won't be present in all browsers for a while. Fall back to exposing the PerShaderStage limit.
        setLimitValueU32('maxStorageBuffersInVertexStage',    compatibilityModeLimitsPtr, {{{ C_STRUCTS.WGPUCompatibilityModeLimits.maxStorageBuffersInVertexStage }}},    limits.maxStorageBuffersPerShaderStage);
        setLimitValueU32('maxStorageBuffersInFragmentStage',  compatibilityModeLimitsPtr, {{{ C_STRUCTS.WGPUCompatibilityModeLimits.maxStorageBuffersInFragmentStage }}},  limits.maxStorageBuffersPerShaderStage);
        setLimitValueU32('maxStorageTexturesInVertexStage',   compatibilityModeLimitsPtr, {{{ C_STRUCTS.WGPUCompatibilityModeLimits.maxStorageTexturesInVertexStage }}},   limits.maxStorageTexturesPerShaderStage);
        setLimitValueU32('maxStorageTexturesInFragmentStage', compatibilityModeLimitsPtr, {{{ C_STRUCTS.WGPUCompatibilityModeLimits.maxStorageTexturesInFragmentStage }}}, limits.maxStorageTexturesPerShaderStage);
      }
    },

    fillAdapterInfoStruct__deps: ['$stringToNewUTF8', '$lengthBytesUTF8'],
    fillAdapterInfoStruct: (info, infoStruct) => {
      {{{ gpu.makeCheckDescriptor('infoStruct') }}}

      // Populate subgroup limits.
      {{{ makeSetValue('infoStruct', C_STRUCTS.WGPUAdapterInfo.subgroupMinSize, 'info.subgroupMinSize', 'u32') }}};
      {{{ makeSetValue('infoStruct', C_STRUCTS.WGPUAdapterInfo.subgroupMaxSize, 'info.subgroupMaxSize', 'u32') }}};

      // Append all the strings together to condense into a single malloc.
      var strs = info.vendor + info.architecture + info.device + info.description;
      var strPtr = stringToNewUTF8(strs);

      var vendorLen = lengthBytesUTF8(info.vendor);
      WebGPU.setStringView(infoStruct + {{{ C_STRUCTS.WGPUAdapterInfo.vendor }}}, strPtr, vendorLen);
      strPtr += vendorLen;

      var architectureLen = lengthBytesUTF8(info.architecture);
      WebGPU.setStringView(infoStruct + {{{ C_STRUCTS.WGPUAdapterInfo.architecture }}}, strPtr, architectureLen);
      strPtr += architectureLen;

      var deviceLen = lengthBytesUTF8(info.device);
      WebGPU.setStringView(infoStruct + {{{ C_STRUCTS.WGPUAdapterInfo.device }}}, strPtr, deviceLen);
      strPtr += deviceLen;

      var descriptionLen = lengthBytesUTF8(info.description);
      WebGPU.setStringView(infoStruct + {{{ C_STRUCTS.WGPUAdapterInfo.description }}}, strPtr, descriptionLen);
      strPtr += descriptionLen;

      {{{ makeSetValue('infoStruct', C_STRUCTS.WGPUAdapterInfo.backendType, gpu.BackendType.WebGPU, 'i32') }}};
      var adapterType = info.isFallbackAdapter ? {{{ gpu.AdapterType.CPU }}} : {{{ gpu.AdapterType.Unknown }}};
      {{{ makeSetValue('infoStruct', C_STRUCTS.WGPUAdapterInfo.adapterType, 'adapterType', 'i32') }}};
      {{{ makeSetValue('infoStruct', C_STRUCTS.WGPUAdapterInfo.vendorID, '0', 'u32') }}};
      {{{ makeSetValue('infoStruct', C_STRUCTS.WGPUAdapterInfo.deviceID, '0', 'u32') }}};
    },

    // Maps from enum number to enum string.
    {{{ WEBGPU_INT_TO_STRING_TABLES }}}
  },

  // Maps from enum string back to enum number, for callbacks.
  {{{ WEBGPU_STRING_TO_INT_TABLES }}}

  // TODO(crbug.com/374150686): Remove this once it has been fully deprecated in users.
  emscripten_webgpu_get_device__deps: ['wgpuDeviceAddRef'],
  emscripten_webgpu_get_device: () => {
#if ASSERTIONS
    assert(Module['preinitializedWebGPUDevice']);
#endif
    if (WebGPU.preinitializedDeviceId === undefined) {
      WebGPU.preinitializedDeviceId = WebGPU.importJsDevice(Module['preinitializedWebGPUDevice']);
      // Some users depend on this keeping the device alive, so we add an
      // additional reference when we first initialize it.
      _wgpuDeviceAddRef(WebGPU.preinitializedDeviceId);
    }
    _wgpuDeviceAddRef(WebGPU.preinitializedDeviceId);
    return WebGPU.preinitializedDeviceId;
  },

  // ----------------------------------------------------------------------------
  // Definitions for standalone JS emwgpu functions (callable from webgpu.cpp and
  //   library_html5_html.js)
  // ----------------------------------------------------------------------------

  emwgpuDelete__sig: 'vp',
  emwgpuDelete: (ptr) => {
    delete WebGPU.Internals.jsObjects[ptr];
  },

  emwgpuSetLabel__deps: ['$UTF8ToString'],
  emwgpuSetLabel__sig: 'vppp',
  emwgpuSetLabel: (ptr, data, length) => {
    var obj = WebGPU.getJsObject(ptr);
    obj.label = UTF8ToString(data, length);
  },

  // Returns a FutureID that was resolved, or kNullFutureId if timed out.
  emwgpuWaitAny__sig: 'dppp',
#if ASYNCIFY
  emwgpuWaitAny__async: true,
  emwgpuWaitAny: (futurePtr, futureCount, timeoutMSPtr) => Asyncify.handleAsync(async () => {
    var promises = [];
    if (timeoutMSPtr) {
      var timeoutMS = {{{ makeGetValue('timeoutMSPtr', 0, 'i32') }}};
      promises.length = futureCount + 1;
      promises[futureCount] = new Promise((resolve) => setTimeout(resolve, timeoutMS, 0));
    } else {
      promises.length = futureCount;
    }

    for (var i = 0; i < futureCount; ++i) {
      // If any FutureID is not tracked, it means it must be done.
      var futureId = {{{ makeGetValue('(futurePtr + i * 8)', 0, 'i53') }}};
      if (!(futureId in WebGPU.Internals.futures)) {
        return futureId;
      }
      promises[i] = WebGPU.Internals.futures[futureId];
    }

    const firstResolvedFuture = await Promise.race(promises);
    delete WebGPU.Internals.futures[firstResolvedFuture];
    return firstResolvedFuture;
  }),
#else
  emwgpuWaitAny: (futurePtr, futureCount, timeoutMSPtr) => {
    abort('TODO: Implement asyncify-free WaitAny for timeout=0');
  },
#endif

  emwgpuGetPreferredFormat__deps: ['$emwgpuStringToInt_PreferredFormat'],
  emwgpuGetPreferredFormat__sig: 'i',
  emwgpuGetPreferredFormat: () => {
    var format = navigator.gpu.getPreferredCanvasFormat();
    return emwgpuStringToInt_PreferredFormat[format];
  },

  // --------------------------------------------------------------------------
  // WebGPU function definitions, with methods organized by "class".
  //
  // Also note that the full set of functions declared in webgpu.h are only
  // partially implemented here. The remaining ones are implemented via
  // webgpu.cpp.
  // --------------------------------------------------------------------------

  // --------------------------------------------------------------------------
  // Standalone (non-method) functions
  // --------------------------------------------------------------------------

  wgpuGetProcAddress: (device, procName) => {
    abort('TODO(#11526): wgpuGetProcAddress unimplemented');
    return {{{ gpu.NULLPTR }}};
  },

  // --------------------------------------------------------------------------
  // Methods of Adapter
  // --------------------------------------------------------------------------

  wgpuAdapterGetFeatures__deps: ['malloc', '$emwgpuStringToInt_FeatureName'],
  wgpuAdapterGetFeatures: (adapterPtr, supportedFeatures) => {
    var adapter = WebGPU.getJsObject(adapterPtr);

    // Always allocate enough space for all the features, though some may be unused.
    var featuresPtr = _malloc(adapter.features.size * 4);
    var offset = 0;
    var numFeatures = 0;
    for (const feature of adapter.features) {
      var featureEnumValue = emwgpuStringToInt_FeatureName[feature];
      if (featureEnumValue >= 0) {
        {{{ makeSetValue('featuresPtr', 'offset', 'featureEnumValue', 'i32') }}};
        offset += 4;
        numFeatures++;
      }
    };
    {{{ makeSetValue('supportedFeatures', C_STRUCTS.WGPUSupportedFeatures.features, 'featuresPtr', '*') }}};
    {{{ makeSetValue('supportedFeatures', C_STRUCTS.WGPUSupportedFeatures.featureCount, 'numFeatures', '*') }}};
  },

  wgpuAdapterGetInfo: (adapterPtr, info) => {
    var adapter = WebGPU.getJsObject(adapterPtr);
    WebGPU.fillAdapterInfoStruct(adapter.info, info);
    return {{{ gpu.Status.Success }}};
  },

  wgpuAdapterGetLimits: (adapterPtr, limitsOutPtr) => {
    var adapter = WebGPU.getJsObject(adapterPtr);
    WebGPU.fillLimitStruct(adapter.limits, limitsOutPtr);
    return {{{ gpu.Status.Success }}};
  },

  wgpuAdapterHasFeature: (adapterPtr, featureEnumValue) => {
    var adapter = WebGPU.getJsObject(adapterPtr);
    return adapter.features.has(WebGPU.FeatureName[featureEnumValue]);
  },

  emwgpuAdapterRequestDevice__deps: [
    'emwgpuOnDeviceLostCompleted',
    'emwgpuOnRequestDeviceCompleted',
    'emwgpuOnUncapturedError',
    '$emwgpuStringToInt_DeviceLostReason',
    '$callUserCallback',
  ],
  emwgpuAdapterRequestDevice__sig: 'vpjjppp',
  emwgpuAdapterRequestDevice: (adapterPtr, futureId, deviceLostFutureId, devicePtr, queuePtr, descriptor) => {
    var adapter = WebGPU.getJsObject(adapterPtr);

    var desc = {};
    if (descriptor) {
      {{{ gpu.makeCheckDescriptor('descriptor') }}}
      var requiredFeatureCount = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUDeviceDescriptor.requiredFeatureCount, '*') }}};
      if (requiredFeatureCount) {
        var requiredFeaturesPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUDeviceDescriptor.requiredFeatures, '*') }}};
        // requiredFeaturesPtr is a pointer to an array of FeatureName which is an enum of size uint32_t
        desc["requiredFeatures"] = Array.from({{{ makeHEAPView('U32', 'requiredFeaturesPtr', `requiredFeaturesPtr + requiredFeatureCount * 4`) }}},
          (feature) => WebGPU.FeatureName[feature]);
      }
      var limitsPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUDeviceDescriptor.requiredLimits, '*') }}};
      if (limitsPtr) {
        {{{ gpu.makeCheck('limitsPtr') }}}
        var nextInChainPtr = {{{ makeGetValue('limitsPtr', C_STRUCTS.WGPULimits.nextInChain, '*') }}};
        var requiredLimits = {};
        function setLimitU32IfDefined(name, basePtr, limitOffset, ignoreIfZero = false) {
          var ptr = basePtr + limitOffset;
          var value = {{{ makeGetValue('ptr', 0, 'u32') }}};
          if (value != {{{ gpu.LIMIT_U32_UNDEFINED }}} && (!ignoreIfZero || value != 0)) {
            requiredLimits[name] = value;
          }
        }
        function setLimitU64IfDefined(name, basePtr, limitOffset) {
          var ptr = basePtr + limitOffset;
          // Handle WGPU_LIMIT_U64_UNDEFINED.
          var limitPart1 = {{{ makeGetValue('ptr', 0, 'u32') }}};
          var limitPart2 = {{{ makeGetValue('ptr', 4, 'u32') }}};
          if (limitPart1 != 0xFFFFFFFF || limitPart2 != 0xFFFFFFFF) {
            requiredLimits[name] = {{{ makeGetValue('ptr', 0, 'i53') }}};
          }
        }

        setLimitU32IfDefined("maxTextureDimension1D",                     limitsPtr, {{{ C_STRUCTS.WGPULimits.maxTextureDimension1D }}});
        setLimitU32IfDefined("maxTextureDimension2D",                     limitsPtr, {{{ C_STRUCTS.WGPULimits.maxTextureDimension2D }}});
        setLimitU32IfDefined("maxTextureDimension3D",                     limitsPtr, {{{ C_STRUCTS.WGPULimits.maxTextureDimension3D }}});
        setLimitU32IfDefined("maxTextureArrayLayers",                     limitsPtr, {{{ C_STRUCTS.WGPULimits.maxTextureArrayLayers }}});
        setLimitU32IfDefined("maxBindGroups",                             limitsPtr, {{{ C_STRUCTS.WGPULimits.maxBindGroups }}});
        setLimitU32IfDefined('maxBindGroupsPlusVertexBuffers',            limitsPtr, {{{ C_STRUCTS.WGPULimits.maxBindGroupsPlusVertexBuffers }}});
        setLimitU32IfDefined('maxBindingsPerBindGroup',                   limitsPtr, {{{ C_STRUCTS.WGPULimits.maxBindingsPerBindGroup }}});
        setLimitU32IfDefined("maxDynamicUniformBuffersPerPipelineLayout", limitsPtr, {{{ C_STRUCTS.WGPULimits.maxDynamicUniformBuffersPerPipelineLayout }}});
        setLimitU32IfDefined("maxDynamicStorageBuffersPerPipelineLayout", limitsPtr, {{{ C_STRUCTS.WGPULimits.maxDynamicStorageBuffersPerPipelineLayout }}});
        setLimitU32IfDefined("maxSampledTexturesPerShaderStage",          limitsPtr, {{{ C_STRUCTS.WGPULimits.maxSampledTexturesPerShaderStage }}});
        setLimitU32IfDefined("maxSamplersPerShaderStage",                 limitsPtr, {{{ C_STRUCTS.WGPULimits.maxSamplersPerShaderStage }}});
        setLimitU32IfDefined("maxStorageBuffersPerShaderStage",           limitsPtr, {{{ C_STRUCTS.WGPULimits.maxStorageBuffersPerShaderStage }}});
        setLimitU32IfDefined("maxStorageTexturesPerShaderStage",          limitsPtr, {{{ C_STRUCTS.WGPULimits.maxStorageTexturesPerShaderStage }}});
        setLimitU32IfDefined("maxUniformBuffersPerShaderStage",           limitsPtr, {{{ C_STRUCTS.WGPULimits.maxUniformBuffersPerShaderStage }}});
        setLimitU32IfDefined("minUniformBufferOffsetAlignment",           limitsPtr, {{{ C_STRUCTS.WGPULimits.minUniformBufferOffsetAlignment }}});
        setLimitU32IfDefined("minStorageBufferOffsetAlignment",           limitsPtr, {{{ C_STRUCTS.WGPULimits.minStorageBufferOffsetAlignment }}});
        setLimitU64IfDefined("maxUniformBufferBindingSize",               limitsPtr, {{{ C_STRUCTS.WGPULimits.maxUniformBufferBindingSize }}});
        setLimitU64IfDefined("maxStorageBufferBindingSize",               limitsPtr, {{{ C_STRUCTS.WGPULimits.maxStorageBufferBindingSize }}});
        setLimitU32IfDefined("maxVertexBuffers",                          limitsPtr, {{{ C_STRUCTS.WGPULimits.maxVertexBuffers }}});
        setLimitU64IfDefined("maxBufferSize",                             limitsPtr, {{{ C_STRUCTS.WGPULimits.maxBufferSize }}});
        setLimitU32IfDefined("maxVertexAttributes",                       limitsPtr, {{{ C_STRUCTS.WGPULimits.maxVertexAttributes }}});
        setLimitU32IfDefined("maxVertexBufferArrayStride",                limitsPtr, {{{ C_STRUCTS.WGPULimits.maxVertexBufferArrayStride }}});
        setLimitU32IfDefined("maxInterStageShaderVariables",              limitsPtr, {{{ C_STRUCTS.WGPULimits.maxInterStageShaderVariables }}});
        setLimitU32IfDefined("maxColorAttachments",                       limitsPtr, {{{ C_STRUCTS.WGPULimits.maxColorAttachments }}});
        setLimitU32IfDefined("maxColorAttachmentBytesPerSample",          limitsPtr, {{{ C_STRUCTS.WGPULimits.maxColorAttachmentBytesPerSample }}});
        setLimitU32IfDefined("maxComputeWorkgroupStorageSize",            limitsPtr, {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupStorageSize }}});
        setLimitU32IfDefined("maxComputeInvocationsPerWorkgroup",         limitsPtr, {{{ C_STRUCTS.WGPULimits.maxComputeInvocationsPerWorkgroup }}});
        setLimitU32IfDefined("maxComputeWorkgroupSizeX",                  limitsPtr, {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupSizeX }}});
        setLimitU32IfDefined("maxComputeWorkgroupSizeY",                  limitsPtr, {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupSizeY }}});
        setLimitU32IfDefined("maxComputeWorkgroupSizeZ",                  limitsPtr, {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupSizeZ }}});
        setLimitU32IfDefined("maxComputeWorkgroupsPerDimension",          limitsPtr, {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupsPerDimension }}});
        // Not present in all browsers. If the app requested 0, avoid passing it through so it won't cause an error.
        setLimitU32IfDefined("maxImmediateSize",                          limitsPtr, {{{ C_STRUCTS.WGPULimits.maxImmediateSize }}}, true);

        if (nextInChainPtr !== 0) {
          var sType = {{{ makeGetValue('nextInChainPtr', C_STRUCTS.WGPUChainedStruct.sType, 'i32') }}};
    #if ASSERTIONS
          assert(sType === {{{ gpu.SType.CompatibilityModeLimits }}});
          assert(0 === {{{ makeGetValue('nextInChainPtr', gpu.kOffsetOfNextInChainMember, '*') }}});
    #endif
          var compatibilityModeLimitsPtr = nextInChainPtr;
          {{{ gpu.makeCheckDescriptor('compatibilityModeLimitsPtr') }}}
          // If not present in the browser, don't request these, otherwise they'll cause an error.
          // (Technically, if any of these is higher than the PerShaderStage equivalent, we should
          // raise the PerShaderStage limit instead, but that's complex and apps should be able to
          // deal with that themselves.)
          if ('maxStorageBuffersInVertexStage' in GPUSupportedLimits.prototype) {
            setLimitU32IfDefined('maxStorageBuffersInVertexStage',    compatibilityModeLimitsPtr, {{{ C_STRUCTS.WGPUCompatibilityModeLimits.maxStorageBuffersInVertexStage }}});
            setLimitU32IfDefined('maxStorageTexturesInVertexStage',   compatibilityModeLimitsPtr, {{{ C_STRUCTS.WGPUCompatibilityModeLimits.maxStorageTexturesInVertexStage }}});
            setLimitU32IfDefined('maxStorageBuffersInFragmentStage',  compatibilityModeLimitsPtr, {{{ C_STRUCTS.WGPUCompatibilityModeLimits.maxStorageBuffersInFragmentStage }}});
            setLimitU32IfDefined('maxStorageTexturesInFragmentStage', compatibilityModeLimitsPtr, {{{ C_STRUCTS.WGPUCompatibilityModeLimits.maxStorageTexturesInFragmentStage }}});
          }
        }

        desc["requiredLimits"] = requiredLimits;
      }

      var defaultQueuePtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUDeviceDescriptor.defaultQueue, '*') }}};
      if (defaultQueuePtr) {
        var defaultQueueDesc = {
          "label": WebGPU.makeStringFromOptionalStringView(
            defaultQueuePtr + {{{ C_STRUCTS.WGPUQueueDescriptor.label }}}),
        };
        desc["defaultQueue"] = defaultQueueDesc;
      }
      desc["label"] = WebGPU.makeStringFromOptionalStringView(
        descriptor + {{{ C_STRUCTS.WGPUDeviceDescriptor.label }}}
      );
    }

    {{{ runtimeKeepalivePush() }}} // requestDevice
    WebGPU.Internals.futureInsert(futureId, adapter.requestDevice(desc).then((device) => {
      {{{ runtimeKeepalivePop() }}} // requestDevice fulfilled
      callUserCallback(() => {
        WebGPU.Internals.jsObjectInsert(queuePtr, device.queue);
        WebGPU.Internals.jsObjectInsert(devicePtr, device);

        {{{ gpu.convertToPassAsPointer('devicePtr') }}}

        // Set up device lost promise resolution.
#if ASSERTIONS
        assert(deviceLostFutureId);
#endif
        // Don't keepalive here, because this isn't guaranteed to ever happen.
        WebGPU.Internals.futureInsert(deviceLostFutureId, device.lost.then((info) => {
          // If the runtime has exited, avoid calling callUserCallback as it
          // will print an error (e.g. if the device got freed during shutdown).
#if EXIT_RUNTIME
          if (runtimeExited) return;
#endif
          callUserCallback(() => {
            // Unset the uncaptured error handler.
            device.onuncapturederror = (ev) => {};
            var sp = stackSave();
            var messagePtr = stringToUTF8OnStack(info.message);
            _emwgpuOnDeviceLostCompleted(deviceLostFutureId, emwgpuStringToInt_DeviceLostReason[info.reason],
              {{{ gpu.passAsPointer('messagePtr') }}});
            stackRestore(sp);
          });
        }));

        // Set up uncaptured error handlers.
#if ASSERTIONS
        assert(typeof GPUValidationError != 'undefined');
        assert(typeof GPUOutOfMemoryError != 'undefined');
        assert(typeof GPUInternalError != 'undefined');
#endif
        device.onuncapturederror = (ev) => {
            var type = {{{ gpu.ErrorType.Unknown }}};
            if (ev.error instanceof GPUValidationError) type = {{{ gpu.ErrorType.Validation }}};
            else if (ev.error instanceof GPUOutOfMemoryError) type = {{{ gpu.ErrorType.OutOfMemory }}};
            else if (ev.error instanceof GPUInternalError) type = {{{ gpu.ErrorType.Internal }}};
            var sp = stackSave();
            var messagePtr = stringToUTF8OnStack(ev.error.message);
            _emwgpuOnUncapturedError({{{ gpu.passAsPointer('devicePtr') }}}, type, {{{ gpu.passAsPointer('messagePtr') }}});
            stackRestore(sp);
        };

        _emwgpuOnRequestDeviceCompleted(futureId, {{{ gpu.RequestDeviceStatus.Success }}},
          {{{ gpu.passAsPointer('devicePtr') }}}, {{{ gpu.NULLPTR }}});
      });
    }, (ex) => {
      {{{ runtimeKeepalivePop() }}} // requestDevice rejected
      callUserCallback(() => {
        var sp = stackSave();
        var messagePtr = stringToUTF8OnStack(ex.message);
        _emwgpuOnRequestDeviceCompleted(futureId, {{{ gpu.RequestDeviceStatus.Error }}},
          {{{ gpu.passAsPointer('devicePtr') }}}, {{{ gpu.passAsPointer('messagePtr') }}});
        if (deviceLostFutureId) {
          _emwgpuOnDeviceLostCompleted(deviceLostFutureId, {{{ gpu.DeviceLostReason.FailedCreation }}},
            {{{ gpu.passAsPointer('messagePtr') }}});
        }
        stackRestore(sp);
      });
    }));
  },

  // --------------------------------------------------------------------------
  // Methods of BindGroup
  // --------------------------------------------------------------------------

  // --------------------------------------------------------------------------
  // Methods of BindGroupLayout
  // --------------------------------------------------------------------------

  // --------------------------------------------------------------------------
  // Methods of Buffer
  // --------------------------------------------------------------------------

  emwgpuBufferDestroy__sig: 'vp',
  emwgpuBufferDestroy: (bufferPtr) => {
    var buffer = WebGPU.getJsObject(bufferPtr);
    var onUnmap = WebGPU.Internals.bufferOnUnmaps[bufferPtr];
    if (onUnmap) {
      for (var i = 0; i < onUnmap.length; ++i) {
        onUnmap[i]();
      }
      delete WebGPU.Internals.bufferOnUnmaps[bufferPtr];
    }

    buffer.destroy();
  },

  emwgpuBufferGetConstMappedRange__deps: ['$warnOnce', 'memalign', 'free'],
  emwgpuBufferGetConstMappedRange__sig: 'pppp',
  emwgpuBufferGetConstMappedRange: (bufferPtr, offset, size) => {
    var buffer = WebGPU.getJsObject(bufferPtr);

#if ASSERTIONS
    if (size === 0) warnOnce('getMappedRange size=0 no longer means WGPU_WHOLE_MAP_SIZE');
#endif

    {{{ gpu.convertSentinelToUndefined('size', '*') }}}

    var mapped;
    try {
      mapped = buffer.getMappedRange(offset, size);
    } catch (ex) {
#if ASSERTIONS
      err(`buffer.getMappedRange(${offset}, ${size}) failed: ${ex}`);
#endif
      return {{{ gpu.NULLPTR }}};
    }
    var data = _memalign(16, mapped.byteLength);
    HEAPU8.set(new Uint8Array(mapped), data);
    WebGPU.Internals.bufferOnUnmaps[bufferPtr].push(() => _free(data));
    return data;
  },

  emwgpuBufferGetMappedRange__deps: ['$warnOnce', 'memalign', 'free'],
  emwgpuBufferGetMappedRange__sig: 'pppp',
  emwgpuBufferGetMappedRange: (bufferPtr, offset, size) => {
    var buffer = WebGPU.getJsObject(bufferPtr);

#if ASSERTIONS
    if (size === 0) warnOnce('getMappedRange size=0 no longer means WGPU_WHOLE_MAP_SIZE');
#endif

    {{{ gpu.convertSentinelToUndefined('size', '*') }}}

    var mapped;
    try {
      mapped = buffer.getMappedRange(offset, size);
    } catch (ex) {
#if ASSERTIONS
      err(`buffer.getMappedRange(${offset}, ${size}) failed: ${ex}`);
#endif
      return {{{ gpu.NULLPTR }}};
    }

    var data = _memalign(16, mapped.byteLength);
    HEAPU8.fill(0, data, mapped.byteLength);
    WebGPU.Internals.bufferOnUnmaps[bufferPtr].push(() => {
      new Uint8Array(mapped).set(HEAPU8.subarray(data, data + mapped.byteLength));
      _free(data);
    });
    return data;
  },

  emwgpuBufferWriteMappedRange__sig: 'ipppp',
  emwgpuBufferWriteMappedRange: (bufferPtr, offset, data, size) => {
    var buffer = WebGPU.getJsObject(bufferPtr);
    var mapped;
    try {
      mapped = buffer.getMappedRange(offset, size);
    } catch (ex) {
#if ASSERTIONS
      err(`buffer.getMappedRange(${offset}, ${size}) failed: ${ex}`);
#endif
      return {{{ gpu.Status.Error }}};
    }
    new Uint8Array(mapped).set(HEAPU8.subarray(data, data + size));
    return {{{ gpu.Status.Success }}};
  },

  emwgpuBufferReadMappedRange__sig: 'ipppp',
  emwgpuBufferReadMappedRange: (bufferPtr, offset, data, size) => {
    var buffer = WebGPU.getJsObject(bufferPtr);
    var mapped;
    try {
      mapped = buffer.getMappedRange(offset, size);
    } catch (ex) {
#if ASSERTIONS
      err(`buffer.getMappedRange(${offset}, ${size}) failed: ${ex}`);
#endif
      return {{{ gpu.Status.Error }}};
    }
    HEAPU8.set(new Uint8Array(mapped), data);
    return {{{ gpu.Status.Success }}};
  },

  wgpuBufferGetSize: (bufferPtr) => {
    var buffer = WebGPU.getJsObject(bufferPtr);
    // 64-bit
    return buffer.size;
  },

  wgpuBufferGetUsage: (bufferPtr) => {
    var buffer = WebGPU.getJsObject(bufferPtr);
    return buffer.usage;
  },

  // In webgpu.h offset and size are passed in as size_t.
  // And library_webgpu assumes that size_t is always 32bit in emscripten.
  emwgpuBufferMapAsync__deps: ['emwgpuOnMapAsyncCompleted', '$callUserCallback'],
  emwgpuBufferMapAsync__sig: 'vpjjpp',
  emwgpuBufferMapAsync: (bufferPtr, futureId, mode, offset, size) => {
    var buffer = WebGPU.getJsObject(bufferPtr);
    WebGPU.Internals.bufferOnUnmaps[bufferPtr] = [];

    {{{ gpu.convertSentinelToUndefined('size', '*') }}}

    {{{ runtimeKeepalivePush() }}} // mapAsync
    WebGPU.Internals.futureInsert(futureId, buffer.mapAsync(mode, offset, size).then(() => {
      {{{ runtimeKeepalivePop() }}} // mapAsync fulfilled
      callUserCallback(() => {
        _emwgpuOnMapAsyncCompleted(futureId, {{{ gpu.MapAsyncStatus.Success }}},
          {{{ gpu.NULLPTR }}});
      });
    }, (ex) => {
      {{{ runtimeKeepalivePop() }}} // mapAsync rejected
      callUserCallback(() => {
        var sp = stackSave();
        var messagePtr = stringToUTF8OnStack(ex.message);
        var status =
          ex.name === 'AbortError' ? {{{ gpu.MapAsyncStatus.Aborted }}} :
          ex.name === 'OperationError' ? {{{ gpu.MapAsyncStatus.Error }}} :
          0;
        {{{ gpu.makeCheck('status') }}}
        _emwgpuOnMapAsyncCompleted(futureId, status, {{{ gpu.passAsPointer('messagePtr') }}});
        delete WebGPU.Internals.bufferOnUnmaps[bufferPtr];
      });
    }));
  },

  emwgpuBufferUnmap__sig: 'vp',
  emwgpuBufferUnmap: (bufferPtr) => {
    var buffer = WebGPU.getJsObject(bufferPtr);

    var onUnmap = WebGPU.Internals.bufferOnUnmaps[bufferPtr];
    if (!onUnmap) {
      // Already unmapped
      return;
    }

    for (var i = 0; i < onUnmap.length; ++i) {
      onUnmap[i]();
    }
    delete WebGPU.Internals.bufferOnUnmaps[bufferPtr]

    buffer.unmap();
  },

  // --------------------------------------------------------------------------
  // Methods of CommandBuffer
  // --------------------------------------------------------------------------

  // --------------------------------------------------------------------------
  // Methods of CommandEncoder
  // --------------------------------------------------------------------------

  wgpuCommandEncoderBeginComputePass__deps: ['emwgpuCreateComputePassEncoder'],
  wgpuCommandEncoderBeginComputePass: (encoderPtr, descriptor) => {
    var desc;

    if (descriptor) {
      {{{ gpu.makeCheckDescriptor('descriptor') }}}
      desc = {
        "label": WebGPU.makeStringFromOptionalStringView(
          descriptor + {{{ C_STRUCTS.WGPUComputePassDescriptor.label }}}),
        "timestampWrites": WebGPU.makePassTimestampWrites(
          {{{ makeGetValue('descriptor', C_STRUCTS.WGPUComputePassDescriptor.timestampWrites, '*') }}}),
      };
    }
    var commandEncoder = WebGPU.getJsObject(encoderPtr);
    var ptr = _emwgpuCreateComputePassEncoder({{{ gpu.NULLPTR }}});
    WebGPU.Internals.jsObjectInsert(ptr, commandEncoder.beginComputePass(desc));
    return ptr;
  },

  wgpuCommandEncoderBeginRenderPass__deps: ['emwgpuCreateRenderPassEncoder'],
  wgpuCommandEncoderBeginRenderPass: (encoderPtr, descriptor) => {
    {{{ gpu.makeCheck('descriptor') }}}

    function makeColorAttachment(caPtr) {
      var viewPtr = {{{ makeGetValue('caPtr', C_STRUCTS.WGPURenderPassColorAttachment.view, '*') }}};
      if (viewPtr === 0) {
        // Null `view` means no attachment in this slot.
        return undefined;
      }

      var depthSlice = {{{ makeGetValue('caPtr', C_STRUCTS.WGPURenderPassColorAttachment.depthSlice, 'u32') }}};
      {{{ gpu.convertSentinelToUndefined('depthSlice', 'u32') }}}

      return {
        "view": WebGPU.getJsObject(viewPtr),
        "depthSlice": depthSlice,
        "resolveTarget": WebGPU.getJsObject(
          {{{ makeGetValue('caPtr', C_STRUCTS.WGPURenderPassColorAttachment.resolveTarget, '*') }}}),
        "clearValue": WebGPU.makeColor(caPtr + {{{ C_STRUCTS.WGPURenderPassColorAttachment.clearValue }}}),
        "loadOp": {{{ gpu.makeGetEnum('caPtr', C_STRUCTS.WGPURenderPassColorAttachment.loadOp, 'LoadOp') }}},
        "storeOp": {{{ gpu.makeGetEnum('caPtr', C_STRUCTS.WGPURenderPassColorAttachment.storeOp, 'StoreOp') }}},
      };
    }

    function makeColorAttachments(count, caPtr) {
      var attachments = [];
      for (var i = 0; i < count; ++i) {
        attachments.push(makeColorAttachment(caPtr + {{{ C_STRUCTS.WGPURenderPassColorAttachment.__size__ }}} * i));
      }
      return attachments;
    }

    function makeDepthStencilAttachment(dsaPtr) {
      if (dsaPtr === 0) return undefined;

      return {
        "view": WebGPU.getJsObject(
          {{{ makeGetValue('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.view, '*') }}}),
        "depthClearValue": {{{ makeGetValue('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.depthClearValue, 'float') }}},
        "depthLoadOp": {{{ gpu.makeGetEnum('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.depthLoadOp, 'LoadOp') }}},
        "depthStoreOp": {{{ gpu.makeGetEnum('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.depthStoreOp, 'StoreOp') }}},
        "depthReadOnly": {{{ gpu.makeGetBool('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.depthReadOnly) }}},
        "stencilClearValue": {{{ makeGetValue('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.stencilClearValue, 'u32') }}},
        "stencilLoadOp": {{{ gpu.makeGetEnum('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.stencilLoadOp, 'LoadOp') }}},
        "stencilStoreOp": {{{ gpu.makeGetEnum('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.stencilStoreOp, 'StoreOp') }}},
        "stencilReadOnly": {{{ gpu.makeGetBool('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.stencilReadOnly) }}},
      };
    }

    function makeRenderPassDescriptor(descriptor) {
      {{{ gpu.makeCheck('descriptor') }}}
      var nextInChainPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPassDescriptor.nextInChain, '*') }}};

      var maxDrawCount = undefined;
      if (nextInChainPtr !== 0) {
        var sType = {{{ makeGetValue('nextInChainPtr', C_STRUCTS.WGPUChainedStruct.sType, 'i32') }}};
#if ASSERTIONS
        assert(sType === {{{ gpu.SType.RenderPassMaxDrawCount }}});
        assert(0 === {{{ makeGetValue('nextInChainPtr', gpu.kOffsetOfNextInChainMember, '*') }}});
#endif
        var renderPassMaxDrawCount = nextInChainPtr;
        {{{ gpu.makeCheckDescriptor('renderPassMaxDrawCount') }}}
        // Note: The user could have passed a really huge value here, which is technically valid in
        // C but will not be allowed by WebGPU in JS because of [EnforceRange]. We intentionally
        // ignore that case because it's not useful - apps can just pick a smaller maxDrawCount.
        maxDrawCount = {{{ makeGetValue('renderPassMaxDrawCount', C_STRUCTS.WGPURenderPassMaxDrawCount.maxDrawCount, 'i53') }}};
      }

      var desc = {
        "label": WebGPU.makeStringFromOptionalStringView(
          descriptor + {{{ C_STRUCTS.WGPURenderPassDescriptor.label }}}),
        "colorAttachments": makeColorAttachments(
          {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPassDescriptor.colorAttachmentCount, '*') }}},
          {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPassDescriptor.colorAttachments, '*') }}}),
        "depthStencilAttachment": makeDepthStencilAttachment(
          {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPassDescriptor.depthStencilAttachment, '*') }}}),
        "occlusionQuerySet": WebGPU.getJsObject(
          {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPassDescriptor.occlusionQuerySet, '*') }}}),
        "timestampWrites": WebGPU.makePassTimestampWrites(
          {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPassDescriptor.timestampWrites, '*') }}}),
        "maxDrawCount": maxDrawCount,
      };
      return desc;
    }

    var desc = makeRenderPassDescriptor(descriptor);

    var commandEncoder = WebGPU.getJsObject(encoderPtr);
    var ptr = _emwgpuCreateRenderPassEncoder({{{ gpu.NULLPTR }}});
    WebGPU.Internals.jsObjectInsert(ptr, commandEncoder.beginRenderPass(desc));
    return ptr;
  },

  wgpuCommandEncoderClearBuffer: (encoderPtr, bufferPtr, offset, size) => {
    var commandEncoder = WebGPU.getJsObject(encoderPtr);
    {{{ gpu.convertSentinelToUndefined('size', 'i53') }}}

    var buffer = WebGPU.getJsObject(bufferPtr);
    commandEncoder.clearBuffer(buffer, offset, size);
  },

  wgpuCommandEncoderCopyBufferToBuffer: (encoderPtr, srcPtr, srcOffset, dstPtr, dstOffset, size) => {
    var commandEncoder = WebGPU.getJsObject(encoderPtr);
    var src = WebGPU.getJsObject(srcPtr);
    var dst = WebGPU.getJsObject(dstPtr);
    commandEncoder.copyBufferToBuffer(src, srcOffset, dst, dstOffset, size);
  },

  wgpuCommandEncoderCopyBufferToTexture: (encoderPtr, srcPtr, dstPtr, copySizePtr) => {
    var commandEncoder = WebGPU.getJsObject(encoderPtr);
    var copySize = WebGPU.makeExtent3D(copySizePtr);
    commandEncoder.copyBufferToTexture(
      WebGPU.makeTexelCopyBufferInfo(srcPtr), WebGPU.makeTexelCopyTextureInfo(dstPtr), copySize);
  },

  wgpuCommandEncoderCopyTextureToBuffer: (encoderPtr, srcPtr, dstPtr, copySizePtr) => {
    var commandEncoder = WebGPU.getJsObject(encoderPtr);
    var copySize = WebGPU.makeExtent3D(copySizePtr);
    commandEncoder.copyTextureToBuffer(
      WebGPU.makeTexelCopyTextureInfo(srcPtr), WebGPU.makeTexelCopyBufferInfo(dstPtr), copySize);
  },

  wgpuCommandEncoderCopyTextureToTexture: (encoderPtr, srcPtr, dstPtr, copySizePtr) => {
    var commandEncoder = WebGPU.getJsObject(encoderPtr);
    var copySize = WebGPU.makeExtent3D(copySizePtr);
    commandEncoder.copyTextureToTexture(
      WebGPU.makeTexelCopyTextureInfo(srcPtr), WebGPU.makeTexelCopyTextureInfo(dstPtr), copySize);
  },

  wgpuCommandEncoderFinish__deps: ['emwgpuCreateCommandBuffer'],
  wgpuCommandEncoderFinish: (encoderPtr, descriptor) => {
    // TODO: Use the descriptor.
    var commandEncoder = WebGPU.getJsObject(encoderPtr);
    var ptr = _emwgpuCreateCommandBuffer({{{ gpu.NULLPTR }}});
    WebGPU.Internals.jsObjectInsert(ptr, commandEncoder.finish());
    return ptr;
  },

  wgpuCommandEncoderInsertDebugMarker: (encoderPtr, markerLabelPtr) => {
    var encoder = WebGPU.getJsObject(encoderPtr);
    encoder.insertDebugMarker(WebGPU.makeStringFromStringView(markerLabelPtr));
  },

  wgpuCommandEncoderPopDebugGroup: (encoderPtr) => {
    var encoder = WebGPU.getJsObject(encoderPtr);
    encoder.popDebugGroup();
  },

  wgpuCommandEncoderPushDebugGroup: (encoderPtr, groupLabelPtr) => {
    var encoder = WebGPU.getJsObject(encoderPtr);
    encoder.pushDebugGroup(WebGPU.makeStringFromStringView(groupLabelPtr));
  },

  wgpuCommandEncoderResolveQuerySet: (encoderPtr, querySetPtr, firstQuery, queryCount, destinationPtr, destinationOffset) => {
    {{{ gpu.convertToU31('firstQuery') }}}
    {{{ gpu.convertToU31('queryCount') }}}
    var commandEncoder = WebGPU.getJsObject(encoderPtr);
    var querySet = WebGPU.getJsObject(querySetPtr);
    var destination = WebGPU.getJsObject(destinationPtr);

    commandEncoder.resolveQuerySet(querySet, firstQuery, queryCount, destination, destinationOffset);
  },

  wgpuCommandEncoderWriteTimestamp: (encoderPtr, querySetPtr, queryIndex) => {
    {{{ gpu.convertToU31('queryIndex') }}}
    var commandEncoder = WebGPU.getJsObject(encoderPtr);
    var querySet = WebGPU.getJsObject(querySetPtr);
    commandEncoder.writeTimestamp(querySet, queryIndex);
  },

  // --------------------------------------------------------------------------
  // Methods of ComputePassEncoder
  // --------------------------------------------------------------------------

  wgpuComputePassEncoderDispatchWorkgroups: (passPtr, x, y, z) => {
    {{{ gpu.convertToU31('x') }}}
    {{{ gpu.convertToU31('y') }}}
    {{{ gpu.convertToU31('z') }}}
    var pass = WebGPU.getJsObject(passPtr);
    pass.dispatchWorkgroups(x, y, z);
  },

  wgpuComputePassEncoderDispatchWorkgroupsIndirect: (passPtr, indirectBufferPtr, indirectOffset) => {
    var indirectBuffer = WebGPU.getJsObject(indirectBufferPtr);
    var pass = WebGPU.getJsObject(passPtr);
    pass.dispatchWorkgroupsIndirect(indirectBuffer, indirectOffset);
  },

  wgpuComputePassEncoderEnd: (passPtr) => {
    var pass = WebGPU.getJsObject(passPtr);
    pass.end();
  },

  wgpuComputePassEncoderInsertDebugMarker: (encoderPtr, markerLabelPtr) => {
    var encoder = WebGPU.getJsObject(encoderPtr);
    encoder.insertDebugMarker(WebGPU.makeStringFromStringView(markerLabelPtr));
  },

  wgpuComputePassEncoderPopDebugGroup: (encoderPtr) => {
    var encoder = WebGPU.getJsObject(encoderPtr);
    encoder.popDebugGroup();
  },

  wgpuComputePassEncoderPushDebugGroup: (encoderPtr, groupLabelPtr) => {
    var encoder = WebGPU.getJsObject(encoderPtr);
    encoder.pushDebugGroup(WebGPU.makeStringFromStringView(groupLabelPtr));
  },

  wgpuComputePassEncoderSetBindGroup: (passPtr, groupIndex, groupPtr, dynamicOffsetCount, dynamicOffsetsPtr) => {
    {{{ gpu.convertToU31('groupIndex') }}}
    var pass = WebGPU.getJsObject(passPtr);
    var group = WebGPU.getJsObject(groupPtr);
    if (dynamicOffsetCount == 0) {
      pass.setBindGroup(groupIndex, group);
    } else {
      pass.setBindGroup(groupIndex, group, HEAPU32, {{{ getHeapOffset('dynamicOffsetsPtr', 'u32') }}}, dynamicOffsetCount);
    }
  },

  wgpuComputePassEncoderSetPipeline: (passPtr, pipelinePtr) => {
    var pass = WebGPU.getJsObject(passPtr);
    var pipeline = WebGPU.getJsObject(pipelinePtr);
    pass.setPipeline(pipeline);
  },

  wgpuComputePassEncoderWriteTimestamp: (encoderPtr, querySetPtr, queryIndex) => {
    {{{ gpu.convertToU31('queryIndex') }}}
    var encoder = WebGPU.getJsObject(encoderPtr);
    var querySet = WebGPU.getJsObject(querySetPtr);
    encoder.writeTimestamp(querySet, queryIndex);
  },

  // --------------------------------------------------------------------------
  // Methods of ComputePipeline
  // --------------------------------------------------------------------------

  wgpuComputePipelineGetBindGroupLayout__deps: ['emwgpuCreateBindGroupLayout'],
  wgpuComputePipelineGetBindGroupLayout: (pipelinePtr, groupIndex) => {
    {{{ gpu.convertToU31('groupIndex') }}}
    var pipeline = WebGPU.getJsObject(pipelinePtr);
    var ptr = _emwgpuCreateBindGroupLayout({{{ gpu.NULLPTR }}});
    WebGPU.Internals.jsObjectInsert(ptr, pipeline.getBindGroupLayout(groupIndex));
    return ptr;
  },

  // --------------------------------------------------------------------------
  // Methods of Device
  // --------------------------------------------------------------------------

  wgpuDeviceCreateBindGroup__deps: ['emwgpuCreateBindGroup'],
  wgpuDeviceCreateBindGroup: (devicePtr, descriptor) => {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}

    function makeEntry(entryPtr) {
      {{{ gpu.makeCheck('entryPtr') }}}

      var bufferPtr = {{{ makeGetValue('entryPtr', C_STRUCTS.WGPUBindGroupEntry.buffer, '*') }}};
      var samplerPtr = {{{ makeGetValue('entryPtr', C_STRUCTS.WGPUBindGroupEntry.sampler, '*') }}};
      var textureViewPtr = {{{ makeGetValue('entryPtr', C_STRUCTS.WGPUBindGroupEntry.textureView, '*') }}};
      var externalTexturePtr = 0;
      WebGPU.iterateExtensions(entryPtr, {
        {{{ gpu.SType.ExternalTextureBindingEntry }}}: (ptr) => {
          externalTexturePtr = {{{ makeGetValue('ptr', C_STRUCTS.WGPUExternalTextureBindingEntry.externalTexture, '*') }}};
        },
      });
#if ASSERTIONS
      assert((bufferPtr !== 0) + (samplerPtr !== 0) + (textureViewPtr !== 0) + (externalTexturePtr !== 0) === 1);
#endif

      var resource;
      if (bufferPtr) {
        // Note the sentinel UINT64_MAX will be read as -1.
        var size = {{{ makeGetValue('entryPtr', C_STRUCTS.WGPUBindGroupEntry.size, 'i53') }}};
        {{{ gpu.convertSentinelToUndefined('size', 'i53') }}}

        resource = {
          "buffer": WebGPU.getJsObject(bufferPtr),
          "offset": {{{ makeGetValue('entryPtr', C_STRUCTS.WGPUBindGroupEntry.offset, 'i53') }}},
          "size": size,
        };
      } else {
        resource = WebGPU.getJsObject(samplerPtr || textureViewPtr || externalTexturePtr);
      }
      return {
        "binding": {{{ makeGetValue('entryPtr', C_STRUCTS.WGPUBindGroupEntry.binding, 'u32') }}},
        "resource": resource,
      };
    }

    function makeEntries(count, entriesPtrs) {
      var entries = [];
      for (var i = 0; i < count; ++i) {
        entries.push(makeEntry(entriesPtrs +
            {{{C_STRUCTS.WGPUBindGroupEntry.__size__}}} * i));
      }
      return entries;
    }

    var desc = {
      "label": WebGPU.makeStringFromOptionalStringView(
        descriptor + {{{ C_STRUCTS.WGPUBindGroupDescriptor.label }}}),
      "layout": WebGPU.getJsObject(
        {{{ makeGetValue('descriptor', C_STRUCTS.WGPUBindGroupDescriptor.layout, '*') }}}),
      "entries": makeEntries(
        {{{ makeGetValue('descriptor', C_STRUCTS.WGPUBindGroupDescriptor.entryCount, '*') }}},
        {{{ makeGetValue('descriptor', C_STRUCTS.WGPUBindGroupDescriptor.entries, '*') }}}
      ),
    };

    var device = WebGPU.getJsObject(devicePtr);
    var ptr = _emwgpuCreateBindGroup({{{ gpu.NULLPTR }}});
    WebGPU.Internals.jsObjectInsert(ptr, device.createBindGroup(desc));
    return ptr;
  },

  wgpuDeviceCreateBindGroupLayout__deps: ['emwgpuCreateBindGroupLayout'],
  wgpuDeviceCreateBindGroupLayout: (devicePtr, descriptor) => {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}

    function makeBufferEntry(substructPtr) {
      var typeInt =
        {{{ makeGetValue('substructPtr', C_STRUCTS.WGPUBufferBindingLayout.type, 'u32') }}};
      if (!typeInt) return undefined;

      return {
        "type": WebGPU.BufferBindingType[typeInt],
        "hasDynamicOffset":
          {{{ gpu.makeGetBool('substructPtr', C_STRUCTS.WGPUBufferBindingLayout.hasDynamicOffset) }}},
        "minBindingSize":
          {{{ makeGetValue('substructPtr', C_STRUCTS.WGPUBufferBindingLayout.minBindingSize, 'i53') }}},
      };
    }

    function makeSamplerEntry(substructPtr) {
      var typeInt =
        {{{ makeGetValue('substructPtr', C_STRUCTS.WGPUSamplerBindingLayout.type, 'u32') }}};
      if (!typeInt) return undefined;

      return {
        "type": WebGPU.SamplerBindingType[typeInt],
      };
    }

    function makeTextureEntry(substructPtr) {
      var sampleTypeInt =
        {{{ makeGetValue('substructPtr', C_STRUCTS.WGPUTextureBindingLayout.sampleType, 'u32') }}};
      if (!sampleTypeInt) return undefined;

      return {
        "sampleType": WebGPU.TextureSampleType[sampleTypeInt],
        "viewDimension": {{{ gpu.makeGetEnum('substructPtr', C_STRUCTS.WGPUTextureBindingLayout.viewDimension, 'TextureViewDimension') }}},
        "multisampled":
          {{{ gpu.makeGetBool('substructPtr', C_STRUCTS.WGPUTextureBindingLayout.multisampled) }}},
      };
    }

    function makeStorageTextureEntry(substructPtr) {
      var accessInt =
        {{{ makeGetValue('substructPtr', C_STRUCTS.WGPUStorageTextureBindingLayout.access, 'u32') }}}
      if (!accessInt) return undefined;

      return {
        "access": WebGPU.StorageTextureAccess[accessInt],
        "format": {{{ gpu.makeGetEnum('substructPtr', C_STRUCTS.WGPUStorageTextureBindingLayout.format, 'TextureFormat') }}},
        "viewDimension": {{{ gpu.makeGetEnum('substructPtr', C_STRUCTS.WGPUStorageTextureBindingLayout.viewDimension, 'TextureViewDimension') }}},
      };
    }

    function makeEntry(entryPtr) {
      {{{ gpu.makeCheck('entryPtr') }}}
#if ASSERTIONS
      // bindingArraySize is not specced and thus not implemented yet. We don't pass it through
      // because if we did, then existing apps using this version of the bindings could break when
      // browsers start accepting bindingArraySize.
      var bindingArraySize = {{{ makeGetValue('entryPtr', C_STRUCTS.WGPUBindGroupLayoutEntry.bindingArraySize, 'u32') }}};
      assert(bindingArraySize == 0 || bindingArraySize == 1);
#endif

      var entry = {
        "binding":
          {{{ makeGetValue('entryPtr', C_STRUCTS.WGPUBindGroupLayoutEntry.binding, 'u32') }}},
        "visibility":
          {{{ makeGetValue('entryPtr', C_STRUCTS.WGPUBindGroupLayoutEntry.visibility, 'u32') }}},
        "buffer": makeBufferEntry(entryPtr + {{{ C_STRUCTS.WGPUBindGroupLayoutEntry.buffer }}}),
        "sampler": makeSamplerEntry(entryPtr + {{{ C_STRUCTS.WGPUBindGroupLayoutEntry.sampler }}}),
        "texture": makeTextureEntry(entryPtr + {{{ C_STRUCTS.WGPUBindGroupLayoutEntry.texture }}}),
        "storageTexture": makeStorageTextureEntry(entryPtr + {{{ C_STRUCTS.WGPUBindGroupLayoutEntry.storageTexture }}}),
      };
      WebGPU.iterateExtensions(entryPtr, {
        {{{ gpu.SType.ExternalTextureBindingLayout }}}: (ptr) => {
          entry["externalTexture"] = {};
        },
      });
      return entry;
    }

    function makeEntries(count, entriesPtrs) {
      var entries = [];
      for (var i = 0; i < count; ++i) {
        entries.push(makeEntry(entriesPtrs +
            {{{ C_STRUCTS.WGPUBindGroupLayoutEntry.__size__ }}} * i));
      }
      return entries;
    }

    var desc = {
      "label": WebGPU.makeStringFromOptionalStringView(
        descriptor + {{{ C_STRUCTS.WGPUBindGroupLayoutDescriptor.label }}}),
      "entries": makeEntries(
        {{{ makeGetValue('descriptor', C_STRUCTS.WGPUBindGroupLayoutDescriptor.entryCount, '*') }}},
        {{{ makeGetValue('descriptor', C_STRUCTS.WGPUBindGroupLayoutDescriptor.entries, '*') }}}
      ),
    };

    var device = WebGPU.getJsObject(devicePtr);
    var ptr = _emwgpuCreateBindGroupLayout({{{ gpu.NULLPTR }}});
    WebGPU.Internals.jsObjectInsert(ptr, device.createBindGroupLayout(desc));
    return ptr;
  },

  emwgpuDeviceCreateBuffer__sig: 'vppp',
  emwgpuDeviceCreateBuffer: (devicePtr, descriptor, bufferPtr) => {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}

    var mappedAtCreation = {{{ gpu.makeGetBool('descriptor', C_STRUCTS.WGPUBufferDescriptor.mappedAtCreation) }}};

    var desc = {
      "label": WebGPU.makeStringFromOptionalStringView(
        descriptor + {{{ C_STRUCTS.WGPUBufferDescriptor.label }}}),
      "usage": {{{ makeGetValue('descriptor', C_STRUCTS.WGPUBufferDescriptor.usage, 'u32') }}},
      "size": {{{ makeGetValue('descriptor', C_STRUCTS.WGPUBufferDescriptor.size, 'i53') }}},
      "mappedAtCreation": mappedAtCreation,
    };

    var device = WebGPU.getJsObject(devicePtr);
    var buffer;
    try {
      buffer = device.createBuffer(desc);
    } catch (ex) {
      // The only exception should be RangeError if mapping at creation ran out of memory.
      {{{ gpu.makeCheck('ex instanceof RangeError') }}}
      {{{ gpu.makeCheck('mappedAtCreation') }}}
#if ASSERTIONS
      err('createBuffer threw:', ex);
#endif
      return false;
    }
    WebGPU.Internals.jsObjectInsert(bufferPtr, buffer);
    if (mappedAtCreation) {
      WebGPU.Internals.bufferOnUnmaps[bufferPtr] = [];
    }
    return true;
  },

  wgpuDeviceCreateCommandEncoder__deps: ['emwgpuCreateCommandEncoder'],
  wgpuDeviceCreateCommandEncoder: (devicePtr, descriptor) => {
    var desc;
    if (descriptor) {
      {{{ gpu.makeCheckDescriptor('descriptor') }}}
      desc = {
        "label": WebGPU.makeStringFromOptionalStringView(
          descriptor + {{{ C_STRUCTS.WGPUCommandEncoderDescriptor.label }}}),
      };
    }
    var device = WebGPU.getJsObject(devicePtr);
    var ptr = _emwgpuCreateCommandEncoder({{{ gpu.NULLPTR }}});
    WebGPU.Internals.jsObjectInsert(ptr, device.createCommandEncoder(desc));
    return ptr;
  },

  wgpuDeviceCreateComputePipeline__deps: ['emwgpuCreateComputePipeline'],
  wgpuDeviceCreateComputePipeline: (devicePtr, descriptor) => {
    var desc = WebGPU.makeComputePipelineDesc(descriptor);
    var device = WebGPU.getJsObject(devicePtr);
    var ptr = _emwgpuCreateComputePipeline({{{ gpu.NULLPTR }}});
    WebGPU.Internals.jsObjectInsert(ptr, device.createComputePipeline(desc));
    return ptr;
  },

  emwgpuDeviceCreateComputePipelineAsync__deps: ['emwgpuOnCreateComputePipelineCompleted', '$callUserCallback'],
  emwgpuDeviceCreateComputePipelineAsync__sig: 'vpjpp',
  emwgpuDeviceCreateComputePipelineAsync: (devicePtr, futureId, descriptor, pipelinePtr) => {
    var desc = WebGPU.makeComputePipelineDesc(descriptor);
    var device = WebGPU.getJsObject(devicePtr);
    {{{ runtimeKeepalivePush() }}} // createComputePipelineAsync
    WebGPU.Internals.futureInsert(futureId, device.createComputePipelineAsync(desc).then((pipeline) => {
      {{{ runtimeKeepalivePop() }}} // createComputePipelineAsync fulfilled
      callUserCallback(() => {
        WebGPU.Internals.jsObjectInsert(pipelinePtr, pipeline);
        _emwgpuOnCreateComputePipelineCompleted(futureId, {{{ gpu.CreatePipelineAsyncStatus.Success }}},
          {{{ gpu.passAsPointer('pipelinePtr') }}}, {{{ gpu.NULLPTR }}});
      });
    }, (pipelineError) => {
      {{{ runtimeKeepalivePop() }}} // createComputePipelineAsync rejected
      callUserCallback(() => {
        var sp = stackSave();
        var messagePtr = stringToUTF8OnStack(pipelineError.message);
        var status =
          pipelineError.reason === 'validation' ? {{{ gpu.CreatePipelineAsyncStatus.ValidationError }}} :
          pipelineError.reason === 'internal' ? {{{ gpu.CreatePipelineAsyncStatus.InternalError }}} :
          0;
        {{{ gpu.makeCheck('status') }}}
        _emwgpuOnCreateComputePipelineCompleted(futureId, status,
          {{{ gpu.passAsPointer('pipelinePtr') }}}, {{{ gpu.passAsPointer('messagePtr') }}});
        stackRestore(sp);
      });
    }));
  },

  wgpuDeviceCreatePipelineLayout__deps: ['emwgpuCreatePipelineLayout'],
  wgpuDeviceCreatePipelineLayout: (devicePtr, descriptor) => {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}
    var bglCount = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUPipelineLayoutDescriptor.bindGroupLayoutCount, '*') }}};
    var bglPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUPipelineLayoutDescriptor.bindGroupLayouts, '*') }}};
    var bgls = [];
    for (var i = 0; i < bglCount; ++i) {
      bgls.push(WebGPU.getJsObject(
        {{{ makeGetValue('bglPtr', `${POINTER_SIZE} * i`, '*') }}}));
    }
    var desc = {
      "label": WebGPU.makeStringFromOptionalStringView(
        descriptor + {{{ C_STRUCTS.WGPUPipelineLayoutDescriptor.label }}}),
      "bindGroupLayouts": bgls,
    };

    var device = WebGPU.getJsObject(devicePtr);
    var ptr = _emwgpuCreatePipelineLayout({{{ gpu.NULLPTR }}});
    WebGPU.Internals.jsObjectInsert(ptr, device.createPipelineLayout(desc));
    return ptr;
  },

  wgpuDeviceCreateQuerySet__deps: ['emwgpuCreateQuerySet'],
  wgpuDeviceCreateQuerySet: (devicePtr, descriptor) => {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}

    var desc = {
      "type": {{{ gpu.makeGetEnum('descriptor', C_STRUCTS.WGPUQuerySetDescriptor.type, 'QueryType') }}},
      "count": {{{ makeGetValue('descriptor', C_STRUCTS.WGPUQuerySetDescriptor.count, 'u32') }}},
    };

    var device = WebGPU.getJsObject(devicePtr);
    var ptr = _emwgpuCreateQuerySet({{{ gpu.NULLPTR }}});
    WebGPU.Internals.jsObjectInsert(ptr, device.createQuerySet(desc));
    return ptr;
  },

  wgpuDeviceCreateRenderBundleEncoder__deps: ['emwgpuCreateRenderBundleEncoder'],
  wgpuDeviceCreateRenderBundleEncoder: (devicePtr, descriptor) => {
    {{{ gpu.makeCheck('descriptor') }}}

    function makeRenderBundleEncoderDescriptor(descriptor) {
      {{{ gpu.makeCheck('descriptor') }}}

      function makeColorFormats(count, formatsPtr) {
        var formats = [];
        for (var i = 0; i < count; ++i, formatsPtr += 4) {
          // format == undefined means no attachment in this slot.
          formats.push({{{ gpu.makeGetEnum('formatsPtr', 0, 'TextureFormat') }}});
        }
        return formats;
      }

      var desc = {
        "label": WebGPU.makeStringFromOptionalStringView(
          descriptor + {{{ C_STRUCTS.WGPURenderBundleEncoderDescriptor.label }}}),
        "colorFormats": makeColorFormats(
          {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderBundleEncoderDescriptor.colorFormatCount, '*') }}},
          {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderBundleEncoderDescriptor.colorFormats, '*') }}}),
        "depthStencilFormat": {{{ gpu.makeGetEnum('descriptor', C_STRUCTS.WGPURenderBundleEncoderDescriptor.depthStencilFormat, 'TextureFormat') }}},
        "sampleCount": {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderBundleEncoderDescriptor.sampleCount, 'u32') }}},
        "depthReadOnly": {{{ gpu.makeGetBool('descriptor', C_STRUCTS.WGPURenderBundleEncoderDescriptor.depthReadOnly) }}},
        "stencilReadOnly": {{{ gpu.makeGetBool('descriptor', C_STRUCTS.WGPURenderBundleEncoderDescriptor.stencilReadOnly) }}},
      };
      return desc;
    }

    var desc = makeRenderBundleEncoderDescriptor(descriptor);
    var device = WebGPU.getJsObject(devicePtr);
    var ptr = _emwgpuCreateRenderBundleEncoder({{{ gpu.NULLPTR }}});
    WebGPU.Internals.jsObjectInsert(ptr, device.createRenderBundleEncoder(desc));
    return ptr;
  },

  wgpuDeviceCreateRenderPipeline__deps: ['emwgpuCreateRenderPipeline'],
  wgpuDeviceCreateRenderPipeline: (devicePtr, descriptor) => {
    var desc = WebGPU.makeRenderPipelineDesc(descriptor);
    var device = WebGPU.getJsObject(devicePtr);
    var ptr = _emwgpuCreateRenderPipeline({{{ gpu.NULLPTR }}});
    WebGPU.Internals.jsObjectInsert(ptr, device.createRenderPipeline(desc));
    return ptr;
  },

  emwgpuDeviceCreateRenderPipelineAsync__deps: ['emwgpuOnCreateRenderPipelineCompleted', '$callUserCallback'],
  emwgpuDeviceCreateRenderPipelineAsync__sig: 'vpjpp',
  emwgpuDeviceCreateRenderPipelineAsync: (devicePtr, futureId, descriptor, pipelinePtr) => {
    var desc = WebGPU.makeRenderPipelineDesc(descriptor);
    var device = WebGPU.getJsObject(devicePtr);
    {{{ runtimeKeepalivePush() }}} // createRenderPipelineAsync
    WebGPU.Internals.futureInsert(futureId, device.createRenderPipelineAsync(desc).then((pipeline) => {
      {{{ runtimeKeepalivePop() }}} // createRenderPipelineAsync fulfilled
      callUserCallback(() => {
        WebGPU.Internals.jsObjectInsert(pipelinePtr, pipeline);
        _emwgpuOnCreateRenderPipelineCompleted(futureId, {{{ gpu.CreatePipelineAsyncStatus.Success }}},
          {{{ gpu.passAsPointer('pipelinePtr') }}}, {{{ gpu.NULLPTR }}});
      });
    }, (pipelineError) => {
      {{{ runtimeKeepalivePop() }}} // createRenderPipelineAsync rejected
      callUserCallback(() => {
        var sp = stackSave();
        var messagePtr = stringToUTF8OnStack(pipelineError.message);
        var status =
          pipelineError.reason === 'validation' ? {{{ gpu.CreatePipelineAsyncStatus.ValidationError }}} :
          pipelineError.reason === 'internal' ? {{{ gpu.CreatePipelineAsyncStatus.InternalError }}} :
          0;
        {{{ gpu.makeCheck('status') }}}
        _emwgpuOnCreateRenderPipelineCompleted(futureId, status,
          {{{ gpu.passAsPointer('pipelinePtr') }}}, {{{ gpu.passAsPointer('messagePtr') }}});
        stackRestore(sp);
      });
    }));
  },

  wgpuDeviceCreateSampler__deps: ['emwgpuCreateSampler'],
  wgpuDeviceCreateSampler: (devicePtr, descriptor) => {
    var desc;
    if (descriptor) {
      {{{ gpu.makeCheckDescriptor('descriptor') }}}

      desc = {
        "label": WebGPU.makeStringFromOptionalStringView(
          descriptor + {{{ C_STRUCTS.WGPUSamplerDescriptor.label }}}),
        "addressModeU": {{{ gpu.makeGetEnum('descriptor', C_STRUCTS.WGPUSamplerDescriptor.addressModeU, 'AddressMode') }}},
        "addressModeV": {{{ gpu.makeGetEnum('descriptor', C_STRUCTS.WGPUSamplerDescriptor.addressModeV, 'AddressMode') }}},
        "addressModeW": {{{ gpu.makeGetEnum('descriptor', C_STRUCTS.WGPUSamplerDescriptor.addressModeW, 'AddressMode') }}},
        "magFilter": {{{ gpu.makeGetEnum('descriptor', C_STRUCTS.WGPUSamplerDescriptor.magFilter, 'FilterMode') }}},
        "minFilter": {{{ gpu.makeGetEnum('descriptor', C_STRUCTS.WGPUSamplerDescriptor.minFilter, 'FilterMode') }}},
        "mipmapFilter": {{{ gpu.makeGetEnum('descriptor', C_STRUCTS.WGPUSamplerDescriptor.mipmapFilter, 'MipmapFilterMode') }}},
        "lodMinClamp": {{{ makeGetValue('descriptor', C_STRUCTS.WGPUSamplerDescriptor.lodMinClamp, 'float') }}},
        "lodMaxClamp": {{{ makeGetValue('descriptor', C_STRUCTS.WGPUSamplerDescriptor.lodMaxClamp, 'float') }}},
        "compare": {{{ gpu.makeGetEnum('descriptor', C_STRUCTS.WGPUSamplerDescriptor.compare, 'CompareFunction') }}},
        "maxAnisotropy": {{{ makeGetValue('descriptor', C_STRUCTS.WGPUSamplerDescriptor.maxAnisotropy, 'u16') }}},
      };
    }

    var device = WebGPU.getJsObject(devicePtr);
    var ptr = _emwgpuCreateSampler({{{ gpu.NULLPTR }}});
    WebGPU.Internals.jsObjectInsert(ptr, device.createSampler(desc));
    return ptr;
  },

  emwgpuDeviceCreateShaderModule__sig: 'vppp',
  emwgpuDeviceCreateShaderModule: (devicePtr, descriptor, shaderModulePtr) => {
    {{{ gpu.makeCheck('descriptor') }}}
    var nextInChainPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUShaderModuleDescriptor.nextInChain, '*') }}};
#if ASSERTIONS
    assert(nextInChainPtr !== 0);
#endif
    var sType = {{{ makeGetValue('nextInChainPtr', C_STRUCTS.WGPUChainedStruct.sType, 'i32') }}};

    var desc = {
      "label": WebGPU.makeStringFromOptionalStringView(
        descriptor + {{{ C_STRUCTS.WGPUShaderModuleDescriptor.label }}}),
      "code": "",
    };

    switch (sType) {
      case {{{ gpu.SType.ShaderSourceWGSL }}}: {
        desc["code"] = WebGPU.makeStringFromStringView(
          nextInChainPtr + {{{ C_STRUCTS.WGPUShaderSourceWGSL.code }}}
        );
        break;
      }
#if ASSERTIONS
      default: abort('unrecognized ShaderModule sType');
#endif
    }

    var device = WebGPU.getJsObject(devicePtr);
    WebGPU.Internals.jsObjectInsert(shaderModulePtr, device.createShaderModule(desc));
  },

  wgpuDeviceCreateTexture__deps: ['emwgpuCreateTexture'],
  wgpuDeviceCreateTexture: (devicePtr, descriptor) => {
    {{{ gpu.makeCheck('descriptor') }}}
    var nextInChainPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUTextureDescriptor.nextInChain, '*') }}};

    var textureBindingViewDimension;
    if (nextInChainPtr !== 0) {
      var sType = {{{ makeGetValue('nextInChainPtr', C_STRUCTS.WGPUChainedStruct.sType, 'i32') }}};
#if ASSERTIONS
      assert(sType === {{{ gpu.SType.TextureBindingViewDimension }}});
      assert(0 === {{{ makeGetValue('nextInChainPtr', gpu.kOffsetOfNextInChainMember, '*') }}});
#endif
      var textureBindingViewDimensionDescriptor = nextInChainPtr;
      {{{ gpu.makeCheckDescriptor('textureBindingViewDimensionDescriptor') }}}
      textureBindingViewDimension = {{{ gpu.makeGetEnum('textureBindingViewDimensionDescriptor',
        C_STRUCTS.WGPUTextureBindingViewDimension.textureBindingViewDimension, 'TextureViewDimension') }}};
    }

    var desc = {
      "label": WebGPU.makeStringFromOptionalStringView(
        descriptor + {{{ C_STRUCTS.WGPUTextureDescriptor.label }}}),
      "size": WebGPU.makeExtent3D(descriptor + {{{ C_STRUCTS.WGPUTextureDescriptor.size }}}),
      "mipLevelCount": {{{ makeGetValue('descriptor', C_STRUCTS.WGPUTextureDescriptor.mipLevelCount, 'u32') }}},
      "sampleCount": {{{ makeGetValue('descriptor', C_STRUCTS.WGPUTextureDescriptor.sampleCount, 'u32') }}},
      "dimension": {{{ gpu.makeGetEnum('descriptor', C_STRUCTS.WGPUTextureDescriptor.dimension, 'TextureDimension') }}},
      "format": {{{ gpu.makeGetEnum('descriptor', C_STRUCTS.WGPUTextureDescriptor.format, 'TextureFormat') }}},
      "usage": {{{ makeGetValue('descriptor', C_STRUCTS.WGPUTextureDescriptor.usage, 'u32') }}},
      "textureBindingViewDimension": textureBindingViewDimension,
    };

    var viewFormatCount = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUTextureDescriptor.viewFormatCount, '*') }}};
    if (viewFormatCount) {
      var viewFormatsPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUTextureDescriptor.viewFormats, '*') }}};
      // viewFormatsPtr pointer to an array of TextureFormat which is an enum of size uint32_t
      desc['viewFormats'] = Array.from({{{ makeHEAPView('32', 'viewFormatsPtr', 'viewFormatsPtr + viewFormatCount * 4') }}},
        format => WebGPU.TextureFormat[format]);
    }

    var device = WebGPU.getJsObject(devicePtr);
    var ptr = _emwgpuCreateTexture({{{ gpu.NULLPTR }}});
    WebGPU.Internals.jsObjectInsert(ptr, device.createTexture(desc));
    return ptr;
  },

  emwgpuDeviceDestroy: (devicePtr) => {
    const device = WebGPU.getJsObject(devicePtr);
    // Remove the onuncapturederror handler which holds a pointer to the WGPUDevice.
    device.onuncapturederror = null;
    device.destroy()
  },

  wgpuDeviceGetFeatures__deps: ['malloc', '$emwgpuStringToInt_FeatureName'],
  wgpuDeviceGetFeatures: (devicePtr, supportedFeatures) => {
    var device = WebGPU.getJsObject(devicePtr);

    // Always allocate enough space for all the features, though some may be unused.
    var featuresPtr = _malloc(device.features.size * 4);
    var offset = 0;
    var numFeatures = 0;
    for (const feature of device.features) {
      var featureEnumValue = emwgpuStringToInt_FeatureName[feature];
      if (featureEnumValue >= 0) {
        {{{ makeSetValue('featuresPtr', 'offset', 'featureEnumValue', 'i32') }}};
        offset += 4;
        numFeatures++;
      }
    };
    {{{ makeSetValue('supportedFeatures', C_STRUCTS.WGPUSupportedFeatures.features, 'featuresPtr', '*') }}};
    {{{ makeSetValue('supportedFeatures', C_STRUCTS.WGPUSupportedFeatures.featureCount, 'numFeatures', '*') }}};
  },

  wgpuDeviceGetLimits: (devicePtr, limitsOutPtr) => {
    var device = WebGPU.getJsObject(devicePtr);
    WebGPU.fillLimitStruct(device.limits, limitsOutPtr);
    return {{{ gpu.Status.Success }}};
  },

  wgpuDeviceHasFeature: (devicePtr, featureEnumValue) => {
    var device = WebGPU.getJsObject(devicePtr);
    return device.features.has(WebGPU.FeatureName[featureEnumValue]);
  },

  wgpuDeviceGetAdapterInfo: (devicePtr, adapterInfo) => {
    var device = WebGPU.getJsObject(devicePtr);
    WebGPU.fillAdapterInfoStruct(device.adapterInfo, adapterInfo);
    return {{{ gpu.Status.Success }}};
  },

  emwgpuDevicePopErrorScope__deps: ['emwgpuOnPopErrorScopeCompleted', '$callUserCallback'],
  emwgpuDevicePopErrorScope__sig: 'vpj',
  emwgpuDevicePopErrorScope: (devicePtr, futureId) => {
    var device = WebGPU.getJsObject(devicePtr);
    {{{ runtimeKeepalivePush() }}} // popErrorScope
    WebGPU.Internals.futureInsert(futureId, device.popErrorScope().then((gpuError) => {
      {{{ runtimeKeepalivePop() }}} // popErrorScope fulfilled
      callUserCallback(() => {
        var type = {{{ gpu.ErrorType.Unknown }}};
        if (!gpuError) type = {{{ gpu.ErrorType.NoError }}};
        else if (gpuError instanceof GPUValidationError) type = {{{ gpu.ErrorType.Validation }}};
        else if (gpuError instanceof GPUOutOfMemoryError) type = {{{ gpu.ErrorType.OutOfMemory }}};
        else if (gpuError instanceof GPUInternalError) type = {{{ gpu.ErrorType.Internal }}};
#if ASSERTIONS
        else assert(false);
#endif
        var sp = stackSave();
        var messagePtr = gpuError ? stringToUTF8OnStack(gpuError.message) : 0;
        _emwgpuOnPopErrorScopeCompleted(futureId,
          {{{ gpu.PopErrorScopeStatus.Success }}}, type,
          {{{ gpu.passAsPointer('messagePtr') }}});
        stackRestore(sp);
      });
    }, (ex) => {
      {{{ runtimeKeepalivePop() }}} // popErrorScope rejected
      callUserCallback(() => {
        var sp = stackSave();
        var messagePtr = stringToUTF8OnStack(ex.message);
        _emwgpuOnPopErrorScopeCompleted(futureId,
          {{{ gpu.PopErrorScopeStatus.Success }}}, {{{ gpu.ErrorType.Unknown }}},
          {{{ gpu.passAsPointer('messagePtr') }}});
        stackRestore(sp);
      });
    }));
  },

  wgpuDevicePushErrorScope: (devicePtr, filter) => {
    var device = WebGPU.getJsObject(devicePtr);
    device.pushErrorScope(WebGPU.ErrorFilter[filter]);
  },

  // --------------------------------------------------------------------------
  // Methods of Instance
  // --------------------------------------------------------------------------

  wgpuInstanceCreateSurface__deps: ['$findCanvasEventTarget', 'emwgpuCreateSurface'],
  wgpuInstanceCreateSurface: (instancePtr, descriptor) => {
    {{{ gpu.makeCheck('descriptor') }}}
    var nextInChainPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUSurfaceDescriptor.nextInChain, '*') }}};
#if ASSERTIONS
    assert(nextInChainPtr !== 0);
    assert({{{ gpu.SType.EmscriptenSurfaceSourceCanvasHTMLSelector }}} ===
      {{{ makeGetValue('nextInChainPtr', C_STRUCTS.WGPUChainedStruct.sType, 'i32') }}});
#endif
    var sourceCanvasHTMLSelector = nextInChainPtr;

    {{{ gpu.makeCheckDescriptor('sourceCanvasHTMLSelector') }}}
    var selectorPtr = {{{ makeGetValue('sourceCanvasHTMLSelector', C_STRUCTS.WGPUEmscriptenSurfaceSourceCanvasHTMLSelector.selector, '*') }}};
    {{{ gpu.makeCheck('selectorPtr') }}}
    var canvas = findCanvasEventTarget(selectorPtr);
#if OFFSCREENCANVAS_SUPPORT
    if (canvas.offscreenCanvas) canvas = canvas.offscreenCanvas;
#endif
    var context = canvas.getContext('webgpu');
#if ASSERTIONS
    assert(context);
#endif
    if (!context) return {{{ gpu.NULLPTR }}};

    context.surfaceLabelWebGPU = WebGPU.makeStringFromOptionalStringView(
      descriptor + {{{ C_STRUCTS.WGPUSurfaceDescriptor.label }}}
    );

    var ptr = _emwgpuCreateSurface({{{ gpu.NULLPTR }}});
    WebGPU.Internals.jsObjectInsert(ptr, context);
    return ptr;
  },

  wgpuInstanceHasWGSLLanguageFeature: (instance, featureEnumValue) => {
    if (!('wgslLanguageFeatures' in navigator.gpu)) {
      return false;
    }
    return navigator.gpu.wgslLanguageFeatures.has(WebGPU.WGSLLanguageFeatureName[featureEnumValue]);
  },

  wgpuInstanceGetWGSLLanguageFeatures: (instance, supportedFeatures) => {
    // Always allocate enough space for all the features, though some may be unused.
    var featuresPtr = _malloc(navigator.gpu.wgslLanguageFeatures.size * 4);
    var offset = 0;
    var numFeatures = 0;
    for (const feature of navigator.gpu.wgslLanguageFeatures) {
      var featureEnumValue = WebGPU.WGSLLanguageFeatureName.indexOf(feature);
      if (featureEnumValue >= 0) {
        {{{ makeSetValue('featuresPtr', 'offset', 'featureEnumValue', 'i32') }}};
        offset += 4;
        numFeatures++;
      }
    };
    {{{ makeSetValue('supportedFeatures', C_STRUCTS.WGPUSupportedWGSLLanguageFeatures.features, 'featuresPtr', '*') }}};
    {{{ makeSetValue('supportedFeatures', C_STRUCTS.WGPUSupportedWGSLLanguageFeatures.featureCount, 'numFeatures', '*') }}};
  },

  emwgpuInstanceRequestAdapter__deps: ['emwgpuOnRequestAdapterCompleted', '$callUserCallback'],
  emwgpuInstanceRequestAdapter__sig: 'vpjpp',
  emwgpuInstanceRequestAdapter: (instancePtr, futureId, options, adapterPtr) => {
    var opts;
    if (options) {
      {{{ gpu.makeCheck('options') }}}
      opts = {
        "featureLevel": {{{ gpu.makeGetEnum('options', C_STRUCTS.WGPURequestAdapterOptions.featureLevel, 'FeatureLevel') }}},
        "powerPreference": {{{ gpu.makeGetEnum('options', C_STRUCTS.WGPURequestAdapterOptions.powerPreference, 'PowerPreference') }}},
        "forceFallbackAdapter":
          {{{ gpu.makeGetBool('options', C_STRUCTS.WGPURequestAdapterOptions.forceFallbackAdapter) }}},
      };

      var nextInChainPtr = {{{ makeGetValue('options', C_STRUCTS.WGPURequestAdapterOptions.nextInChain, '*') }}};
      if (nextInChainPtr !== 0) {
        var sType = {{{ makeGetValue('nextInChainPtr', C_STRUCTS.WGPUChainedStruct.sType, 'i32') }}};
#if ASSERTIONS
        assert(sType === {{{ gpu.SType.RequestAdapterWebXROptions }}});
        assert(0 === {{{ makeGetValue('nextInChainPtr', gpu.kOffsetOfNextInChainMember, '*') }}});
#endif
        var webxrOptions = nextInChainPtr;
        {{{ gpu.makeCheckDescriptor('webxrOptions') }}}
        opts.xrCompatible = {{{ gpu.makeGetBool('webxrOptions', C_STRUCTS.WGPURequestAdapterWebXROptions.xrCompatible) }}};
      }
    }

    if (!('gpu' in navigator)) {
      var sp = stackSave();
      var messagePtr = stringToUTF8OnStack('WebGPU not available on this browser (navigator.gpu is not available)');
      _emwgpuOnRequestAdapterCompleted(futureId, {{{ gpu.RequestAdapterStatus.Unavailable }}},
        {{{ gpu.passAsPointer('adapterPtr') }}}, {{{ gpu.passAsPointer('messagePtr') }}});
      stackRestore(sp);
      return;
    }

    {{{ runtimeKeepalivePush() }}} // requestAdapter
    WebGPU.Internals.futureInsert(futureId, navigator.gpu.requestAdapter(opts).then((adapter) => {
      {{{ runtimeKeepalivePop() }}} // requestAdapter fulfilled
      callUserCallback(() => {
        if (adapter) {
          WebGPU.Internals.jsObjectInsert(adapterPtr, adapter);
          _emwgpuOnRequestAdapterCompleted(futureId, {{{ gpu.RequestAdapterStatus.Success }}},
            {{{ gpu.passAsPointer('adapterPtr') }}}, {{{ gpu.NULLPTR }}});
        } else {
          var sp = stackSave();
          var messagePtr = stringToUTF8OnStack('WebGPU not available on this browser (requestAdapter returned null)');
          _emwgpuOnRequestAdapterCompleted(futureId, {{{ gpu.RequestAdapterStatus.Unavailable }}},
            {{{ gpu.passAsPointer('adapterPtr') }}}, {{{ gpu.passAsPointer('messagePtr') }}});
          stackRestore(sp);
        }
      });
    }, (ex) => {
      {{{ runtimeKeepalivePop() }}} // requestAdapter rejected
      callUserCallback(() => {
        var sp = stackSave();
        var messagePtr = stringToUTF8OnStack(ex.message);
        _emwgpuOnRequestAdapterCompleted(futureId, {{{ gpu.RequestAdapterStatus.Error }}},
          {{{ gpu.passAsPointer('adapterPtr') }}}, {{{ gpu.passAsPointer('messagePtr') }}});
        stackRestore(sp);
      });
    }));
  },

  // --------------------------------------------------------------------------
  // Methods of PipelineLayout
  // --------------------------------------------------------------------------

  // --------------------------------------------------------------------------
  // Methods of QuerySet
  // --------------------------------------------------------------------------

  wgpuQuerySetDestroy: (querySetPtr) => {
    WebGPU.getJsObject(querySetPtr).destroy();
  },

  wgpuQuerySetGetCount: (querySetPtr) => {
    var querySet = WebGPU.getJsObject(querySetPtr);
    return querySet.count;
  },

  wgpuQuerySetGetType: (querySetPtr) => {
    var querySet = WebGPU.getJsObject(querySetPtr);
    return WebGPU.QueryType.indexOf(querySet.type);
  },

  // --------------------------------------------------------------------------
  // Methods of Queue
  // --------------------------------------------------------------------------

  emwgpuQueueOnSubmittedWorkDone__deps: ['emwgpuOnWorkDoneCompleted', '$callUserCallback'],
  emwgpuQueueOnSubmittedWorkDone__sig: 'vpj',
  emwgpuQueueOnSubmittedWorkDone: (queuePtr, futureId) => {
    var queue = WebGPU.getJsObject(queuePtr);

    {{{ runtimeKeepalivePush() }}} // onSubmittedWorkDone
    WebGPU.Internals.futureInsert(futureId, queue.onSubmittedWorkDone().then(() => {
      {{{ runtimeKeepalivePop() }}} // onSubmittedWorkDone fulfilled (assumed not to reject)
      callUserCallback(() => {
        _emwgpuOnWorkDoneCompleted(futureId, {{{ gpu.QueueWorkDoneStatus.Success }}});
      });
    }));
  },

  wgpuQueueSubmit: (queuePtr, commandCount, commands) => {
#if ASSERTIONS
    assert(commands % 4 === 0);
#endif
    var queue = WebGPU.getJsObject(queuePtr);
    var cmds = Array.from({{{ makeHEAPView(`${POINTER_BITS}`, 'commands', `commands + commandCount * ${POINTER_SIZE}`)}}},
      (id) => WebGPU.getJsObject(id));
    queue.submit(cmds);
  },

  wgpuQueueWriteBuffer: (queuePtr, bufferPtr, bufferOffset, data, size) => {
    var queue = WebGPU.getJsObject(queuePtr);
    var buffer = WebGPU.getJsObject(bufferPtr);
    // There is a size limitation for ArrayBufferView. Work around by passing in a subarray
    // instead of the whole heap. crbug.com/1201109
    var subarray = HEAPU8.subarray(data, data + size);
    queue.writeBuffer(buffer, bufferOffset, subarray, 0, size);
  },

  wgpuQueueWriteTexture: (queuePtr, destinationPtr, data, dataSize, dataLayoutPtr, writeSizePtr) => {
    var queue = WebGPU.getJsObject(queuePtr);

    var destination = WebGPU.makeTexelCopyTextureInfo(destinationPtr);
    var dataLayout = WebGPU.makeTexelCopyBufferLayout(dataLayoutPtr);
    var writeSize = WebGPU.makeExtent3D(writeSizePtr);
    // This subarray isn't strictly necessary, but helps work around an issue
    // where Chromium makes a copy of the entire heap. crbug.com/1134457
    var subarray = HEAPU8.subarray(data, data + dataSize);
    queue.writeTexture(destination, subarray, dataLayout, writeSize);
  },

  // --------------------------------------------------------------------------
  // Methods of RenderBundle
  // --------------------------------------------------------------------------

  // --------------------------------------------------------------------------
  // Methods of RenderBundleEncoder
  // --------------------------------------------------------------------------

  wgpuRenderBundleEncoderDraw: (passPtr, vertexCount, instanceCount, firstVertex, firstInstance) => {
    {{{ gpu.convertToU31('vertexCount') }}}
    {{{ gpu.convertToU31('instanceCount') }}}
    {{{ gpu.convertToU32('firstVertex') }}}
    {{{ gpu.convertToU32('firstInstance') }}}
    var pass = WebGPU.getJsObject(passPtr);
    pass.draw(vertexCount, instanceCount, firstVertex, firstInstance);
  },

  wgpuRenderBundleEncoderDrawIndexed: (passPtr, indexCount, instanceCount, firstIndex, baseVertex, firstInstance) => {
    {{{ gpu.convertToU31('indexCount') }}}
    {{{ gpu.convertToU31('instanceCount') }}}
    {{{ gpu.convertToU32('firstIndex') }}}
    {{{ gpu.convertToU32('firstInstance') }}}
    var pass = WebGPU.getJsObject(passPtr);
    pass.drawIndexed(indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
  },

  wgpuRenderBundleEncoderDrawIndexedIndirect: (passPtr, indirectBufferPtr, indirectOffset) => {
    var indirectBuffer = WebGPU.getJsObject(indirectBufferPtr);
    var pass = WebGPU.getJsObject(passPtr);
    pass.drawIndexedIndirect(indirectBuffer, indirectOffset);
  },

  wgpuRenderBundleEncoderDrawIndirect: (passPtr, indirectBufferPtr, indirectOffset) => {
    var indirectBuffer = WebGPU.getJsObject(indirectBufferPtr);
    var pass = WebGPU.getJsObject(passPtr);
    pass.drawIndirect(indirectBuffer, indirectOffset);
  },

  wgpuRenderBundleEncoderFinish__deps: ['emwgpuCreateRenderBundle'],
  wgpuRenderBundleEncoderFinish: (encoderPtr, descriptor) => {
    var desc;
    if (descriptor) {
      {{{ gpu.makeCheckDescriptor('descriptor') }}}
      desc = {
        "label": WebGPU.makeStringFromOptionalStringView(
          descriptor + {{{ C_STRUCTS.WGPURenderBundleDescriptor.label }}}),
      };
    }
    var encoder = WebGPU.getJsObject(encoderPtr);
    var ptr = _emwgpuCreateRenderBundle({{{ gpu.NULLPTR }}});
    WebGPU.Internals.jsObjectInsert(ptr, encoder.finish(desc));
    return ptr;
  },

  wgpuRenderBundleEncoderInsertDebugMarker: (encoderPtr, markerLabelPtr) => {
    var encoder = WebGPU.getJsObject(encoderPtr);
    encoder.insertDebugMarker(WebGPU.makeStringFromStringView(markerLabelPtr));
  },

  wgpuRenderBundleEncoderPopDebugGroup: (encoderPtr) => {
    var encoder = WebGPU.getJsObject(encoderPtr);
    encoder.popDebugGroup();
  },

  wgpuRenderBundleEncoderPushDebugGroup: (encoderPtr, groupLabelPtr) => {
    var encoder = WebGPU.getJsObject(encoderPtr);
    encoder.pushDebugGroup(WebGPU.makeStringFromStringView(groupLabelPtr));
  },

  wgpuRenderBundleEncoderSetBindGroup: (passPtr, groupIndex, groupPtr, dynamicOffsetCount, dynamicOffsetsPtr) => {
    {{{ gpu.convertToU31('groupIndex') }}}
    var pass = WebGPU.getJsObject(passPtr);
    var group = WebGPU.getJsObject(groupPtr);
    if (dynamicOffsetCount == 0) {
      pass.setBindGroup(groupIndex, group);
    } else {
      pass.setBindGroup(groupIndex, group, HEAPU32, {{{ getHeapOffset('dynamicOffsetsPtr', 'u32') }}}, dynamicOffsetCount);
    }
  },

  wgpuRenderBundleEncoderSetIndexBuffer: (passPtr, bufferPtr, format, offset, size) => {
    var pass = WebGPU.getJsObject(passPtr);
    var buffer = WebGPU.getJsObject(bufferPtr);
    {{{ gpu.convertSentinelToUndefined('size', 'i53') }}}
    pass.setIndexBuffer(buffer, WebGPU.IndexFormat[format], offset, size);
  },

  wgpuRenderBundleEncoderSetPipeline: (passPtr, pipelinePtr) => {
    var pass = WebGPU.getJsObject(passPtr);
    var pipeline = WebGPU.getJsObject(pipelinePtr);
    pass.setPipeline(pipeline);
  },

  wgpuRenderBundleEncoderSetVertexBuffer: (passPtr, slot, bufferPtr, offset, size) => {
    {{{ gpu.convertToU31('slot') }}}
    var pass = WebGPU.getJsObject(passPtr);
    var buffer = WebGPU.getJsObject(bufferPtr);
    {{{ gpu.convertSentinelToUndefined('size', 'i53') }}}
    pass.setVertexBuffer(slot, buffer, offset, size);
  },

  // --------------------------------------------------------------------------
  // Methods of RenderPassEncoder
  // --------------------------------------------------------------------------

  wgpuRenderPassEncoderBeginOcclusionQuery: (passPtr, queryIndex) => {
    {{{ gpu.convertToU31('queryIndex') }}}
    var pass = WebGPU.getJsObject(passPtr);
    pass.beginOcclusionQuery(queryIndex);
  },

  wgpuRenderPassEncoderDraw: (passPtr, vertexCount, instanceCount, firstVertex, firstInstance) => {
    {{{ gpu.convertToU31('vertexCount') }}}
    {{{ gpu.convertToU31('instanceCount') }}}
    {{{ gpu.convertToU32('firstVertex') }}}
    {{{ gpu.convertToU32('firstInstance') }}}
    var pass = WebGPU.getJsObject(passPtr);
    pass.draw(vertexCount, instanceCount, firstVertex, firstInstance);
  },

  wgpuRenderPassEncoderDrawIndexed: (passPtr, indexCount, instanceCount, firstIndex, baseVertex, firstInstance) => {
    {{{ gpu.convertToU31('indexCount') }}}
    {{{ gpu.convertToU31('instanceCount') }}}
    {{{ gpu.convertToU32('firstIndex') }}}
    {{{ gpu.convertToU32('firstInstance') }}}
    var pass = WebGPU.getJsObject(passPtr);
    pass.drawIndexed(indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
  },

  wgpuRenderPassEncoderDrawIndirect: (passPtr, indirectBufferPtr, indirectOffset) => {
    var pass = WebGPU.getJsObject(passPtr);
    var indirectBuffer = WebGPU.getJsObject(indirectBufferPtr);
    pass.drawIndirect(indirectBuffer, indirectOffset);
  },

  wgpuRenderPassEncoderDrawIndexedIndirect: (passPtr, indirectBufferPtr, indirectOffset) => {
    var pass = WebGPU.getJsObject(passPtr);
    var indirectBuffer = WebGPU.getJsObject(indirectBufferPtr);
    pass.drawIndexedIndirect(indirectBuffer, indirectOffset);
  },

  wgpuRenderPassEncoderMultiDrawIndirect: (passPtr, indirectBufferPtr, indirectOffset, maxDrawCount, drawCountBufferPtr, drawCountBufferOffset) => {
    {{{ gpu.convertToU31('maxDrawCount') }}}
    var pass = WebGPU.getJsObject(passPtr);
    var indirectBuffer = WebGPU.getJsObject(indirectBufferPtr);
    var drawCountBuffer = WebGPU.getJsObject(drawCountBufferPtr);
    pass.multiDrawIndirect(indirectBuffer, indirectOffset, maxDrawCount, drawCountBuffer, drawCountBufferOffset);
  },

  wgpuRenderPassEncoderMultiDrawIndexedIndirect: (passPtr, indirectBufferPtr, indirectOffset, maxDrawCount, drawCountBufferPtr, drawCountBufferOffset) => {
    {{{ gpu.convertToU31('maxDrawCount') }}}
    var pass = WebGPU.getJsObject(passPtr);
    var indirectBuffer = WebGPU.getJsObject(indirectBufferPtr);
    var drawCountBuffer = WebGPU.getJsObject(drawCountBufferPtr);
    pass.multiDrawIndexedIndirect(indirectBuffer, indirectOffset, maxDrawCount, drawCountBuffer, drawCountBufferOffset);
  },

  wgpuRenderPassEncoderEnd: (encoderPtr) => {
    var encoder = WebGPU.getJsObject(encoderPtr);
    encoder.end();
  },

  wgpuRenderPassEncoderEndOcclusionQuery: (passPtr) => {
    var pass = WebGPU.getJsObject(passPtr);
    pass.endOcclusionQuery();
  },

  wgpuRenderPassEncoderExecuteBundles: (passPtr, count, bundlesPtr) => {
    var pass = WebGPU.getJsObject(passPtr);

#if ASSERTIONS
    assert(bundlesPtr % 4 === 0);
#endif

    var bundles = Array.from({{{ makeHEAPView(`${POINTER_BITS}`, 'bundlesPtr', `bundlesPtr + count * ${POINTER_SIZE}`) }}},
      (id) => WebGPU.getJsObject(id));
    pass.executeBundles(bundles);
  },

  wgpuRenderPassEncoderInsertDebugMarker: (encoderPtr, markerLabelPtr) => {
    var encoder = WebGPU.getJsObject(encoderPtr);
    encoder.insertDebugMarker(WebGPU.makeStringFromStringView(markerLabelPtr));
  },

  wgpuRenderPassEncoderPopDebugGroup: (encoderPtr) => {
    var encoder = WebGPU.getJsObject(encoderPtr);
    encoder.popDebugGroup();
  },

  wgpuRenderPassEncoderPushDebugGroup: (encoderPtr, groupLabelPtr) => {
    var encoder = WebGPU.getJsObject(encoderPtr);
    encoder.pushDebugGroup(WebGPU.makeStringFromStringView(groupLabelPtr));
  },

  wgpuRenderPassEncoderSetBindGroup: (passPtr, groupIndex, groupPtr, dynamicOffsetCount, dynamicOffsetsPtr) => {
    {{{ gpu.convertToU31('groupIndex') }}}
    var pass = WebGPU.getJsObject(passPtr);
    var group = WebGPU.getJsObject(groupPtr);
    if (dynamicOffsetCount == 0) {
      pass.setBindGroup(groupIndex, group);
    } else {
      pass.setBindGroup(groupIndex, group, HEAPU32, {{{ getHeapOffset('dynamicOffsetsPtr', 'u32') }}}, dynamicOffsetCount);
    }
  },

  wgpuRenderPassEncoderSetBlendConstant: (passPtr, colorPtr) => {
    var pass = WebGPU.getJsObject(passPtr);
    var color = WebGPU.makeColor(colorPtr);
    pass.setBlendConstant(color);
  },

  wgpuRenderPassEncoderSetIndexBuffer: (passPtr, bufferPtr, format, offset, size) => {
    var pass = WebGPU.getJsObject(passPtr);
    var buffer = WebGPU.getJsObject(bufferPtr);
    {{{ gpu.convertSentinelToUndefined('size', 'i53') }}}
    pass.setIndexBuffer(buffer, WebGPU.IndexFormat[format], offset, size);
  },

  wgpuRenderPassEncoderSetPipeline: (passPtr, pipelinePtr) => {
    var pass = WebGPU.getJsObject(passPtr);
    var pipeline = WebGPU.getJsObject(pipelinePtr);
    pass.setPipeline(pipeline);
  },

  wgpuRenderPassEncoderSetScissorRect: (passPtr, x, y, w, h) => {
    {{{ gpu.convertToU31('x') }}}
    {{{ gpu.convertToU31('y') }}}
    {{{ gpu.convertToU31('w') }}}
    {{{ gpu.convertToU31('h') }}}
    var pass = WebGPU.getJsObject(passPtr);
    pass.setScissorRect(x, y, w, h);
  },

  wgpuRenderPassEncoderSetStencilReference: (passPtr, reference) => {
    {{{ gpu.convertToU32('reference') }}}
    var pass = WebGPU.getJsObject(passPtr);
    pass.setStencilReference(reference);
  },

  wgpuRenderPassEncoderSetVertexBuffer: (passPtr, slot, bufferPtr, offset, size) => {
    {{{ gpu.convertToU31('slot') }}}
    var pass = WebGPU.getJsObject(passPtr);
    var buffer = WebGPU.getJsObject(bufferPtr);
    {{{ gpu.convertSentinelToUndefined('size', 'i53') }}}
    pass.setVertexBuffer(slot, buffer, offset, size);
  },

  wgpuRenderPassEncoderSetViewport: (passPtr, x, y, w, h, minDepth, maxDepth) => {
    var pass = WebGPU.getJsObject(passPtr);
    pass.setViewport(x, y, w, h, minDepth, maxDepth);
  },

  wgpuRenderPassEncoderWriteTimestamp: (encoderPtr, querySetPtr, queryIndex) => {
    {{{ gpu.convertToU31('queryIndex') }}}
    var encoder = WebGPU.getJsObject(encoderPtr);
    var querySet = WebGPU.getJsObject(querySetPtr);
    encoder.writeTimestamp(querySet, queryIndex);
  },

  // --------------------------------------------------------------------------
  // Methods of RenderPipeline
  // --------------------------------------------------------------------------

  wgpuRenderPipelineGetBindGroupLayout__deps: ['emwgpuCreateBindGroupLayout'],
  wgpuRenderPipelineGetBindGroupLayout: (pipelinePtr, groupIndex) => {
    {{{ gpu.convertToU31('groupIndex') }}}
    var pipeline = WebGPU.getJsObject(pipelinePtr);
    var ptr = _emwgpuCreateBindGroupLayout({{{ gpu.NULLPTR }}});
    WebGPU.Internals.jsObjectInsert(ptr, pipeline.getBindGroupLayout(groupIndex));
    return ptr;
  },

  // --------------------------------------------------------------------------
  // Methods of Sampler
  // --------------------------------------------------------------------------

  // --------------------------------------------------------------------------
  // Methods of ShaderModule
  // --------------------------------------------------------------------------

  emwgpuShaderModuleGetCompilationInfo__deps: [
    'emwgpuOnCompilationInfoCompleted',
    '$stringToUTF8',
    '$lengthBytesUTF8',
    'malloc',
    '$emwgpuStringToInt_CompilationMessageType',
    '$callUserCallback',
  ],
  emwgpuShaderModuleGetCompilationInfo__sig: 'vpjp',
  emwgpuShaderModuleGetCompilationInfo: (shaderModulePtr, futureId, compilationInfoPtr) => {
    var shaderModule = WebGPU.getJsObject(shaderModulePtr);
    {{{ runtimeKeepalivePush() }}} // getCompilationInfo
    WebGPU.Internals.futureInsert(futureId, shaderModule.getCompilationInfo().then((compilationInfo) => {
      {{{ runtimeKeepalivePop() }}} // getCompilationInfo fulfilled (assumed not to reject)
      callUserCallback(() => {
        const messageCount = compilationInfo.messages.length;
        {{{ makeSetValue('compilationInfoPtr', C_STRUCTS.WGPUCompilationInfo.messageCount, 'messageCount', '*') }}}

        // If there are messages, allocate and initialize them.
        // TODO(crbug.com/377760848): This giant if-block makes the function hard to read. See if
        // there's a way to factor out the initialization of compilationInfoPtr without increasing
        // code size significantly.
        if (messageCount) {
          // Calculate the total length of strings and offsets here to malloc them
          // all at once. Note that we start at 1 instead of 0 for the total size
          // to ensure there's enough space for the null terminator that is always
          // added by stringToUTF8.
          var totalMessagesSize = 1;
          var messageLengths = [];
          for (var i = 0; i < messageCount; ++i) {
            var messageLength = lengthBytesUTF8(compilationInfo.messages[i].message);
            totalMessagesSize += messageLength;
            messageLengths.push(messageLength);
          }
          var messagesPtr = _malloc(totalMessagesSize);

          // Allocate space for all WGPUCompilationMessage values.
          var compilationMessagesPtr = _malloc({{{ C_STRUCTS.WGPUCompilationMessage.__size__ }}} * messageCount);
          {{{ makeSetValue('compilationInfoPtr', C_STRUCTS.WGPUCompilationInfo.messages, 'compilationMessagesPtr', '*') }}};
          // Allocate space for all WGPUDawnCompilationMessageUtf16 values.
          var utf16sPtr = _malloc({{{ C_STRUCTS.WGPUDawnCompilationMessageUtf16.__size__ }}} * messageCount);
          // Fill in the arrays and link the pointers.
          for (var i = 0; i < messageCount; ++i) {
            var compilationMessage = compilationInfo.messages[i];
            var compilationMessagePtr = compilationMessagesPtr + {{{ C_STRUCTS.WGPUCompilationMessage.__size__ }}} * i;
            var utf16Ptr = utf16sPtr + {{{ C_STRUCTS.WGPUDawnCompilationMessageUtf16.__size__ }}} * i;

            // Write out the values to the CompilationMessage.
            WebGPU.setStringView(compilationMessagePtr + {{{ C_STRUCTS.WGPUCompilationMessage.message }}}, messagesPtr, messageLengths[i]);
            // TODO(crbug.com/435488557): Convert JavaScript's UTF-16-code-unit offsets to
            // UTF-8-code-unit offsets. https://github.com/webgpu-native/webgpu-headers/issues/246
            {{{ makeSetValue('compilationMessagePtr', C_STRUCTS.WGPUCompilationMessage.nextInChain, 'utf16Ptr', '*') }}};
            {{{ makeSetValue('compilationMessagePtr', C_STRUCTS.WGPUCompilationMessage.type,    'emwgpuStringToInt_CompilationMessageType[compilationMessage.type]', 'i32') }}};
            {{{ makeSetValue('compilationMessagePtr', C_STRUCTS.WGPUCompilationMessage.lineNum, 'compilationMessage.lineNum', 'i64') }}};
            {{{ makeSetValue('compilationMessagePtr', C_STRUCTS.WGPUCompilationMessage.linePos, 'compilationMessage.linePos', 'i64') }}};
            {{{ makeSetValue('compilationMessagePtr', C_STRUCTS.WGPUCompilationMessage.offset,  'compilationMessage.offset', 'i64') }}};
            {{{ makeSetValue('compilationMessagePtr', C_STRUCTS.WGPUCompilationMessage.length,  'compilationMessage.length', 'i64') }}};

            {{{ makeSetValue('utf16Ptr', C_STRUCTS.WGPUChainedStruct.next, '0', '*') }}};
            {{{ makeSetValue('utf16Ptr', C_STRUCTS.WGPUChainedStruct.sType, gpu.SType.DawnCompilationMessageUtf16, 'i32') }}};
            {{{ makeSetValue('utf16Ptr', C_STRUCTS.WGPUDawnCompilationMessageUtf16.linePos, 'compilationMessage.linePos', 'i64') }}};
            {{{ makeSetValue('utf16Ptr', C_STRUCTS.WGPUDawnCompilationMessageUtf16.offset,  'compilationMessage.offset', 'i64') }}};
            {{{ makeSetValue('utf16Ptr', C_STRUCTS.WGPUDawnCompilationMessageUtf16.length,  'compilationMessage.length', 'i64') }}};

            // Write the string out to the allocated buffer. Note we have to add 1
            // to the length of the string to ensure enough space for the null
            // terminator. However, we only increment the pointer by the exact
            // length so we overwrite the null terminators except for the last one.
            stringToUTF8(compilationMessage.message, messagesPtr, messageLengths[i] + 1);
            messagesPtr += messageLengths[i];
          }
        }

        _emwgpuOnCompilationInfoCompleted(futureId, {{{ gpu.CompilationInfoRequestStatus.Success }}},
          {{{ gpu.passAsPointer('compilationInfoPtr') }}});
      });
    }, () => {
      abort('Unexpected failure in GPUShaderModule.getCompilationInfo().')
    }));
  },

  // --------------------------------------------------------------------------
  // Methods of Surface
  // --------------------------------------------------------------------------

  wgpuSurfaceConfigure: (surfacePtr, config) => {
    {{{ gpu.makeCheck('config') }}}
    var context = WebGPU.getJsObject(surfacePtr);

#if ASSERTIONS
    var presentMode = {{{ makeGetValue('config', C_STRUCTS.WGPUSurfaceConfiguration.presentMode, 'u32') }}};
    assert(presentMode === {{{ gpu.PresentMode.Fifo }}} ||
           presentMode === {{{ gpu.PresentMode.Undefined }}});
#endif

    var canvasSize = [
      {{{ makeGetValue('config', C_STRUCTS.WGPUSurfaceConfiguration.width, 'u32') }}},
      {{{ makeGetValue('config', C_STRUCTS.WGPUSurfaceConfiguration.height, 'u32') }}}
    ];

    if (canvasSize[0] !== 0) {
      context["canvas"]["width"] = canvasSize[0];
    }

    if (canvasSize[1] !== 0) {
      context["canvas"]["height"] = canvasSize[1];
    }

    var configuration = {
      "device": WebGPU.getJsObject({{{ makeGetValue('config', C_STRUCTS.WGPUSurfaceConfiguration.device, '*') }}}),
      "format": {{{ gpu.makeGetEnum('config', C_STRUCTS.WGPUSurfaceConfiguration.format, 'TextureFormat') }}},
      "usage": {{{ makeGetValue('config', C_STRUCTS.WGPUSurfaceConfiguration.usage, 'u32') }}},
      "alphaMode": {{{ gpu.makeGetEnum('config', C_STRUCTS.WGPUSurfaceConfiguration.alphaMode, 'CompositeAlphaMode') }}},
    };

    var viewFormatCount = {{{ makeGetValue('config', C_STRUCTS.WGPUSurfaceConfiguration.viewFormatCount, '*') }}};
    if (viewFormatCount) {
      var viewFormatsPtr = {{{ makeGetValue('config', C_STRUCTS.WGPUSurfaceConfiguration.viewFormats, '*') }}};
      // viewFormatsPtr pointer to an array of TextureFormat which is an enum of size uint32_t
      configuration['viewFormats'] = Array.from({{{ makeHEAPView('32', 'viewFormatsPtr', 'viewFormatsPtr + viewFormatCount * 4') }}},
        format => WebGPU.TextureFormat[format]);
    }

    {
      var nextInChainPtr = {{{ makeGetValue('config', C_STRUCTS.WGPUSurfaceConfiguration.nextInChain, '*') }}};

      if (nextInChainPtr !== 0) {
        var sType = {{{ makeGetValue('nextInChainPtr', C_STRUCTS.WGPUChainedStruct.sType, 'i32') }}};
#if ASSERTIONS
        assert(sType === {{{ gpu.SType.SurfaceColorManagement }}});
        assert(0 === {{{ makeGetValue('nextInChainPtr', gpu.kOffsetOfNextInChainMember, '*') }}});
#endif
        var surfaceColorManagement = nextInChainPtr;
        {{{ gpu.makeCheckDescriptor('surfaceColorManagement') }}}
        configuration.colorSpace = {{{ gpu.makeGetEnum('surfaceColorManagement', C_STRUCTS.WGPUSurfaceColorManagement.colorSpace, 'PredefinedColorSpace') }}};
        configuration.toneMapping = {
          mode: {{{ gpu.makeGetEnum('surfaceColorManagement', C_STRUCTS.WGPUSurfaceColorManagement.toneMappingMode, 'ToneMappingMode') }}},
        };
      }
    }

    context.configure(configuration);
  },

  wgpuSurfaceGetCurrentTexture__deps: ['emwgpuCreateTexture'],
  wgpuSurfaceGetCurrentTexture: (surfacePtr, surfaceTexturePtr) => {
    {{{ gpu.makeCheck('surfaceTexturePtr') }}}
    var context = WebGPU.getJsObject(surfacePtr);

    try {
      var texturePtr = _emwgpuCreateTexture({{{ gpu.NULLPTR }}});
      WebGPU.Internals.jsObjectInsert(texturePtr, context.getCurrentTexture());
      {{{ makeSetValue('surfaceTexturePtr', C_STRUCTS.WGPUSurfaceTexture.texture, 'texturePtr', '*') }}};
      {{{ makeSetValue('surfaceTexturePtr', C_STRUCTS.WGPUSurfaceTexture.status,
        gpu.SurfaceGetCurrentTextureStatus.SuccessOptimal, 'i32') }}};
    } catch (ex) {
#if ASSERTIONS
      err(`wgpuSurfaceGetCurrentTexture() failed: ${ex}`);
#endif
      {{{ makeSetValue('surfaceTexturePtr', C_STRUCTS.WGPUSurfaceTexture.texture, '0', '*') }}};
      {{{ makeSetValue('surfaceTexturePtr', C_STRUCTS.WGPUSurfaceTexture.status,
        gpu.SurfaceGetCurrentTextureStatus.Error, 'i32') }}};
    }
  },

  wgpuSurfacePresent: (surfacePtr) => {
    // TODO: This could probably be emulated with ASYNCIFY.
    abort('wgpuSurfacePresent is unsupported (use requestAnimationFrame via html5.h instead)');
  },

  wgpuSurfaceUnconfigure: (surfacePtr) => {
    var context = WebGPU.getJsObject(surfacePtr);
    context.unconfigure();
  },

  // --------------------------------------------------------------------------
  // Methods of Texture
  // --------------------------------------------------------------------------

  wgpuTextureCreateView__deps: ['emwgpuCreateTextureView'],
  wgpuTextureCreateView: (texturePtr, descriptor) => {
    var desc;
    if (descriptor) {
      var swizzle;
      var nextInChainPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.nextInChain, '*') }}};
      if (nextInChainPtr !== 0) {
        var sType = {{{ makeGetValue('nextInChainPtr', C_STRUCTS.WGPUChainedStruct.sType, 'i32') }}};
#if ASSERTIONS
        assert(sType === {{{ gpu.SType.TextureComponentSwizzleDescriptor }}});
        assert(0 === {{{ makeGetValue('nextInChainPtr', gpu.kOffsetOfNextInChainMember, '*') }}});
#endif
        var swizzleDescriptor = nextInChainPtr;
        {{{ gpu.makeCheckDescriptor('swizzleDescriptor') }}}
        var swizzlePtr = swizzleDescriptor + {{{ C_STRUCTS.WGPUTextureComponentSwizzleDescriptor.swizzle }}};
        var r = {{{ gpu.makeGetEnum('swizzlePtr', C_STRUCTS.WGPUTextureComponentSwizzle.r, 'ComponentSwizzle') }}} || 'r';
        var g = {{{ gpu.makeGetEnum('swizzlePtr', C_STRUCTS.WGPUTextureComponentSwizzle.g, 'ComponentSwizzle') }}} || 'g';
        var b = {{{ gpu.makeGetEnum('swizzlePtr', C_STRUCTS.WGPUTextureComponentSwizzle.b, 'ComponentSwizzle') }}} || 'b';
        var a = {{{ gpu.makeGetEnum('swizzlePtr', C_STRUCTS.WGPUTextureComponentSwizzle.a, 'ComponentSwizzle') }}} || 'a';
        swizzle = `${r}${g}${b}${a}`;
      }

      var mipLevelCount = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.mipLevelCount, 'u32') }}};
      var arrayLayerCount = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.arrayLayerCount, 'u32') }}};
      desc = {
        "label": WebGPU.makeStringFromOptionalStringView(
          descriptor + {{{ C_STRUCTS.WGPUTextureViewDescriptor.label }}}),
        "format": {{{ gpu.makeGetEnum('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.format, 'TextureFormat') }}},
        "dimension": {{{ gpu.makeGetEnum('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.dimension, 'TextureViewDimension') }}},
        "baseMipLevel": {{{ makeGetValue('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.baseMipLevel, 'u32') }}},
        "mipLevelCount": mipLevelCount === {{{ gpu.MIP_LEVEL_COUNT_UNDEFINED }}} ? undefined : mipLevelCount,
        "baseArrayLayer": {{{ makeGetValue('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.baseArrayLayer, 'u32') }}},
        "arrayLayerCount": arrayLayerCount === {{{ gpu.ARRAY_LAYER_COUNT_UNDEFINED }}} ? undefined : arrayLayerCount,
        "aspect": {{{ gpu.makeGetEnum('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.aspect, 'TextureAspect') }}},
        "usage": {{{ makeGetValue('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.usage, 'u32') }}},
        "swizzle": swizzle,
      };
    }

    var texture = WebGPU.getJsObject(texturePtr);
    var ptr = _emwgpuCreateTextureView({{{ gpu.NULLPTR }}});
    WebGPU.Internals.jsObjectInsert(ptr, texture.createView(desc));
    return ptr;
  },

  wgpuTextureDestroy: (texturePtr) => {
    WebGPU.getJsObject(texturePtr).destroy();
  },

  wgpuTextureGetDepthOrArrayLayers: (texturePtr) => {
    var texture = WebGPU.getJsObject(texturePtr);
    return texture.depthOrArrayLayers;
  },

  wgpuTextureGetDimension: (texturePtr) => {
    var texture = WebGPU.getJsObject(texturePtr);
    return WebGPU.TextureDimension.indexOf(texture.dimension);
  },

  wgpuTextureGetFormat: (texturePtr) => {
    var texture = WebGPU.getJsObject(texturePtr);
    // Should return the enum integer instead of string.
    return WebGPU.TextureFormat.indexOf(texture.format);
  },

  wgpuTextureGetHeight: (texturePtr) => {
    var texture = WebGPU.getJsObject(texturePtr);
    return texture.height;
  },

  wgpuTextureGetMipLevelCount: (texturePtr) => {
    var texture = WebGPU.getJsObject(texturePtr);
    return texture.mipLevelCount;
  },

  wgpuTextureGetSampleCount: (texturePtr) => {
    var texture = WebGPU.getJsObject(texturePtr);
    return texture.sampleCount;
  },

  wgpuTextureGetTextureBindingViewDimension: (texturePtr) => {
    var texture = WebGPU.getJsObject(texturePtr);
    if (!texture.textureBindingViewDimension) {
      return {{{ gpu.TextureViewDimension.Undefined }}};
    }
    return WebGPU.TextureViewDimension.indexOf(texture.textureBindingViewDimension);
  },

  wgpuTextureGetUsage: (texturePtr) => {
    var texture = WebGPU.getJsObject(texturePtr);
    return texture.usage;
  },

  wgpuTextureGetWidth: (texturePtr) => {
    var texture = WebGPU.getJsObject(texturePtr);
    return texture.width;
  },

  // --------------------------------------------------------------------------
  // Methods of TextureView
  // --------------------------------------------------------------------------
};

// Add and set __i53abi to true for functions with 64-bit value in their
// signatures, if not explicitly set otherwise.
for (const key of Object.keys(LibraryWebGPU)) {
  if (typeof LibraryWebGPU[key] !== 'function') continue;
  if (key + '__i53abi' in LibraryWebGPU) continue;
  const sigKey = key + '__sig';
  const sig = LibraryWebGPU[sigKey] ? LibraryWebGPU[sigKey] : LibraryManager.library[sigKey];
  if (!sig?.includes('j')) continue;
  LibraryWebGPU[key + '__i53abi'] = true;
}

// Based on autoAddDeps, this helper iterates the object and moves the
// deps upwards into targetDeps and remove them from the object.
function moveDeps(object, targetDeps) {
  for (var item in object) {
    if (!item.endsWith('__deps')) {
      if (object[item + '__deps']) {
        targetDeps.push(...object[item + '__deps']);
        delete object[item + '__deps']
      }
    }
  }
}
// TODO(crbug.com/377760848): Investigate whether Closure is able
// to dead-code-eliminate these; if not, make them library-level items.
moveDeps(LibraryWebGPU.$WebGPU, LibraryWebGPU.$WebGPU__deps)

autoAddDeps(LibraryWebGPU, '$WebGPU');
autoAddDeps(LibraryWebGPU, '$readI53FromI64');
mergeInto(LibraryManager.library, LibraryWebGPU);
