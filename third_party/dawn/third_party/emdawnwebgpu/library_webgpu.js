/**
 * @license
 * Copyright 2019 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

/*
 * Dawn's fork of Emscripten's WebGPU bindings. This will be contributed back to
 * Emscripten after reaching approximately webgpu.h "1.0".
 *
 * IMPORTANT: See //src/emdawnwebgpu/README.md for info on how to use this.
 */

{{{
  if (USE_WEBGPU || !__HAVE_EMDAWNWEBGPU_STRUCT_INFO || !__HAVE_EMDAWNWEBGPU_ENUM_TABLES || !__HAVE_EMDAWNWEBGPU_SIG_INFO) {
    throw new Error("To use Dawn's library_webgpu.js, disable -sUSE_WEBGPU and first include Dawn's library_webgpu_struct_info.js and library_webgpu_enum_tables.js (before library_webgpu.js)");
  }

  // Helper functions for code generation
  globalThis.gpu = {
    convertSentinelToUndefined: function(name, argIsSizeT) {
      // The sentinel value SIZE_MAX is passed as a "p" (pointer) arg, so it comes through as
      // either `0xFFFFFFFF` or `-1` depending on whether `CAN_ADDRESS_2GB` is enabled.
      if (CAN_ADDRESS_2GB && argIsSizeT) {
        // Note CAN_ADDRESS_2GB is always false for MEMORY64 builds.
        return `if (${name} == 0xFFFFFFFF) ${name} = undefined;`;
      } else {
        return `if (${name} == -1) ${name} = undefined;`;
      }
    },

    NULLPTR: MEMORY64 ? '0n' : '0',
    kOffsetOfNextInChainMember: 0,
    passAsI64: value => WASM_BIGINT ? `BigInt(${value})` : value,
    passAsPointer: value => MEMORY64 ? `BigInt(${value})` : value,
    convertToPassAsPointer: variable => MEMORY64 ? `${variable} = BigInt(${variable});` : '',

    makeGetBool: function(struct, offset) {
      return `!!(${makeGetValue(struct, offset, 'u32')})`;
    },
    makeGetU32: function(struct, offset) {
      return makeGetValue(struct, offset, 'u32');
    },
    makeGetU64: function(struct, offset) {
      var l = makeGetValue(struct, offset, 'u32');
      var h = makeGetValue(`(${struct} + 4)`, offset, 'u32')
      return `(${h} * 0x100000000 + ${l})`
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
#if ASSERTIONS
      assert(ptr in WebGPU.Internals.jsObjects);
#endif
      return WebGPU.Internals.jsObjects[ptr];
    },
    {{{ gpu.makeImportJsObject('Adapter') }}}
    {{{ gpu.makeImportJsObject('BindGroup') }}}
    {{{ gpu.makeImportJsObject('BindGroupLayout') }}}
    importJsBuffer__deps: ['emwgpuCreateBuffer'],
    importJsBuffer: (buffer, parentPtr = 0) => {
      // At the moment, we do not allow importing pending buffers.
      assert(buffer.mapState != "pending");
      var mapState = buffer.mapState == "mapped" ?
        {{{ gpu.BufferMapState.Mapped }}} :
        {{{ gpu.BufferMapState.Unmapped }}};
      var bufferPtr = _emwgpuCreateBuffer(parentPtr, mapState);
      WebGPU.Internals.jsObjectInsert(bufferPtr, buffer);
      if (buffer.mapState == "mapped") {
        WebGPU.Internals.bufferOnUnmaps[bufferPtr] = [];
      }
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
    {{{ gpu.makeImportJsObject('BindGroup') }}}
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
        "width": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUExtent3D.width) }}},
        "height": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUExtent3D.height) }}},
        "depthOrArrayLayers": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUExtent3D.depthOrArrayLayers) }}},
      };
    },

    makeOrigin3D: (ptr) => {
      return {
        "x": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUOrigin3D.x) }}},
        "y": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUOrigin3D.y) }}},
        "z": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUOrigin3D.z) }}},
      };
    },

    makeTexelCopyTextureInfo: (ptr) => {
      {{{ gpu.makeCheck('ptr') }}}
      return {
        "texture": WebGPU.getJsObject(
          {{{ makeGetValue('ptr', C_STRUCTS.WGPUTexelCopyTextureInfo.texture, '*') }}}),
        "mipLevel": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUTexelCopyTextureInfo.mipLevel) }}},
        "origin": WebGPU.makeOrigin3D(ptr + {{{ C_STRUCTS.WGPUTexelCopyTextureInfo.origin }}}),
        "aspect": WebGPU.TextureAspect[{{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUTexelCopyTextureInfo.aspect) }}}],
      };
    },

    makeTexelCopyBufferLayout: (ptr) => {
      var bytesPerRow = {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUTexelCopyBufferLayout.bytesPerRow) }}};
      var rowsPerImage = {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUTexelCopyBufferLayout.rowsPerImage) }}};
      return {
        "offset": {{{ gpu.makeGetU64('ptr', C_STRUCTS.WGPUTexelCopyBufferLayout.offset) }}},
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
        "beginningOfPassWriteIndex": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUPassTimestampWrites.beginningOfPassWriteIndex) }}},
        "endOfPassWriteIndex": {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUPassTimestampWrites.endOfPassWriteIndex) }}},
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
          {{{ gpu.makeGetU32('ptr', C_STRUCTS.WGPUComputeState.constantCount) }}},
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
          "topology": WebGPU.PrimitiveTopology[
            {{{ gpu.makeGetU32('psPtr', C_STRUCTS.WGPUPrimitiveState.topology) }}}],
          "stripIndexFormat": WebGPU.IndexFormat[
            {{{ gpu.makeGetU32('psPtr', C_STRUCTS.WGPUPrimitiveState.stripIndexFormat) }}}],
          "frontFace": WebGPU.FrontFace[
            {{{ gpu.makeGetU32('psPtr', C_STRUCTS.WGPUPrimitiveState.frontFace) }}}],
          "cullMode": WebGPU.CullMode[
            {{{ gpu.makeGetU32('psPtr', C_STRUCTS.WGPUPrimitiveState.cullMode) }}}],
          "unclippedDepth":
            {{{ gpu.makeGetBool('psPtr', C_STRUCTS.WGPUPrimitiveState.unclippedDepth) }}},
        };
      }

      function makeBlendComponent(bdPtr) {
        if (!bdPtr) return undefined;
        return {
          "operation": WebGPU.BlendOperation[
            {{{ gpu.makeGetU32('bdPtr', C_STRUCTS.WGPUBlendComponent.operation) }}}],
          "srcFactor": WebGPU.BlendFactor[
            {{{ gpu.makeGetU32('bdPtr', C_STRUCTS.WGPUBlendComponent.srcFactor) }}}],
          "dstFactor": WebGPU.BlendFactor[
            {{{ gpu.makeGetU32('bdPtr', C_STRUCTS.WGPUBlendComponent.dstFactor) }}}],
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
        var formatInt = {{{ gpu.makeGetU32('csPtr', C_STRUCTS.WGPUColorTargetState.format) }}};
        return formatInt === {{{ gpu.TextureFormat.Undefined }}} ? undefined : {
          "format": WebGPU.TextureFormat[formatInt],
          "blend": makeBlendState({{{ makeGetValue('csPtr', C_STRUCTS.WGPUColorTargetState.blend, '*') }}}),
          "writeMask": {{{ gpu.makeGetU32('csPtr', C_STRUCTS.WGPUColorTargetState.writeMask) }}},
        };
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
          "compare": WebGPU.CompareFunction[
            {{{ gpu.makeGetU32('ssfPtr', C_STRUCTS.WGPUStencilFaceState.compare) }}}],
          "failOp": WebGPU.StencilOperation[
            {{{ gpu.makeGetU32('ssfPtr', C_STRUCTS.WGPUStencilFaceState.failOp) }}}],
          "depthFailOp": WebGPU.StencilOperation[
            {{{ gpu.makeGetU32('ssfPtr', C_STRUCTS.WGPUStencilFaceState.depthFailOp) }}}],
          "passOp": WebGPU.StencilOperation[
            {{{ gpu.makeGetU32('ssfPtr', C_STRUCTS.WGPUStencilFaceState.passOp) }}}],
        };
      }

      function makeDepthStencilState(dssPtr) {
        if (!dssPtr) return undefined;

        {{{ gpu.makeCheck('dssPtr') }}}
        return {
          "format": WebGPU.TextureFormat[
            {{{ gpu.makeGetU32('dssPtr', C_STRUCTS.WGPUDepthStencilState.format) }}}],
          "depthWriteEnabled": {{{ gpu.makeGetBool('dssPtr', C_STRUCTS.WGPUDepthStencilState.depthWriteEnabled) }}},
          "depthCompare": WebGPU.CompareFunction[
            {{{ gpu.makeGetU32('dssPtr', C_STRUCTS.WGPUDepthStencilState.depthCompare) }}}],
          "stencilFront": makeStencilStateFace(dssPtr + {{{ C_STRUCTS.WGPUDepthStencilState.stencilFront }}}),
          "stencilBack": makeStencilStateFace(dssPtr + {{{ C_STRUCTS.WGPUDepthStencilState.stencilBack }}}),
          "stencilReadMask": {{{ gpu.makeGetU32('dssPtr', C_STRUCTS.WGPUDepthStencilState.stencilReadMask) }}},
          "stencilWriteMask": {{{ gpu.makeGetU32('dssPtr', C_STRUCTS.WGPUDepthStencilState.stencilWriteMask) }}},
          "depthBias": {{{ makeGetValue('dssPtr', C_STRUCTS.WGPUDepthStencilState.depthBias, 'i32') }}},
          "depthBiasSlopeScale": {{{ makeGetValue('dssPtr', C_STRUCTS.WGPUDepthStencilState.depthBiasSlopeScale, 'float') }}},
          "depthBiasClamp": {{{ makeGetValue('dssPtr', C_STRUCTS.WGPUDepthStencilState.depthBiasClamp, 'float') }}},
        };
      }

      function makeVertexAttribute(vaPtr) {
        {{{ gpu.makeCheck('vaPtr') }}}
        return {
          "format": WebGPU.VertexFormat[
            {{{ gpu.makeGetU32('vaPtr', C_STRUCTS.WGPUVertexAttribute.format) }}}],
          "offset": {{{ gpu.makeGetU64('vaPtr', C_STRUCTS.WGPUVertexAttribute.offset) }}},
          "shaderLocation": {{{ gpu.makeGetU32('vaPtr', C_STRUCTS.WGPUVertexAttribute.shaderLocation) }}},
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
        var stepModeInt = {{{ gpu.makeGetU32('vbPtr', C_STRUCTS.WGPUVertexBufferLayout.stepMode) }}};
        var attributeCountInt = {{{ gpu.makeGetU32('vbPtr', C_STRUCTS.WGPUVertexBufferLayout.attributeCount) }}};
        if (stepModeInt === {{{ gpu.VertexStepMode.Undefined }}} && attributeCountInt === 0) {
          return null;
        }
        return {
          "arrayStride": {{{ gpu.makeGetU64('vbPtr', C_STRUCTS.WGPUVertexBufferLayout.arrayStride) }}},
          "stepMode": WebGPU.VertexStepMode[stepModeInt],
          "attributes": makeVertexAttributes(
            attributeCountInt,
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
            {{{ gpu.makeGetU32('viPtr', C_STRUCTS.WGPUVertexState.constantCount) }}},
            {{{ makeGetValue('viPtr', C_STRUCTS.WGPUVertexState.constants, '*') }}}),
          "buffers": makeVertexBuffers(
            {{{ gpu.makeGetU32('viPtr', C_STRUCTS.WGPUVertexState.bufferCount) }}},
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
          "count": {{{ gpu.makeGetU32('msPtr', C_STRUCTS.WGPUMultisampleState.count) }}},
          "mask": {{{ gpu.makeGetU32('msPtr', C_STRUCTS.WGPUMultisampleState.mask) }}},
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
            {{{ gpu.makeGetU32('fsPtr', C_STRUCTS.WGPUFragmentState.constantCount) }}},
            {{{ makeGetValue('fsPtr', C_STRUCTS.WGPUFragmentState.constants, '*') }}}),
          "targets": makeColorStates(
            {{{ gpu.makeGetU32('fsPtr', C_STRUCTS.WGPUFragmentState.targetCount) }}},
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

    fillLimitStruct: (limits, limitsOutPtr) => {
      {{{ gpu.makeCheckDescriptor('limitsOutPtr') }}}

      function setLimitValueU32(name, limitOffset) {
        var limitValue = limits[name];
        {{{ makeSetValue('limitsOutPtr', 'limitOffset', 'limitValue', 'i32') }}};
      }
      function setLimitValueU64(name, limitOffset) {
        var limitValue = limits[name];
        {{{ makeSetValue('limitsOutPtr', 'limitOffset', 'limitValue', 'i64') }}};
      }

      setLimitValueU32('maxTextureDimension1D', {{{ C_STRUCTS.WGPULimits.maxTextureDimension1D }}});
      setLimitValueU32('maxTextureDimension2D', {{{ C_STRUCTS.WGPULimits.maxTextureDimension2D }}});
      setLimitValueU32('maxTextureDimension3D', {{{ C_STRUCTS.WGPULimits.maxTextureDimension3D }}});
      setLimitValueU32('maxTextureArrayLayers', {{{ C_STRUCTS.WGPULimits.maxTextureArrayLayers }}});
      setLimitValueU32('maxBindGroups', {{{ C_STRUCTS.WGPULimits.maxBindGroups }}});
      setLimitValueU32('maxBindGroupsPlusVertexBuffers', {{{ C_STRUCTS.WGPULimits.maxBindGroupsPlusVertexBuffers }}});
      setLimitValueU32('maxBindingsPerBindGroup', {{{ C_STRUCTS.WGPULimits.maxBindingsPerBindGroup }}});
      setLimitValueU32('maxDynamicUniformBuffersPerPipelineLayout', {{{ C_STRUCTS.WGPULimits.maxDynamicUniformBuffersPerPipelineLayout }}});
      setLimitValueU32('maxDynamicStorageBuffersPerPipelineLayout', {{{ C_STRUCTS.WGPULimits.maxDynamicStorageBuffersPerPipelineLayout }}});
      setLimitValueU32('maxSampledTexturesPerShaderStage', {{{ C_STRUCTS.WGPULimits.maxSampledTexturesPerShaderStage }}});
      setLimitValueU32('maxSamplersPerShaderStage', {{{ C_STRUCTS.WGPULimits.maxSamplersPerShaderStage }}});
      setLimitValueU32('maxStorageBuffersPerShaderStage', {{{ C_STRUCTS.WGPULimits.maxStorageBuffersPerShaderStage }}});
      setLimitValueU32('maxStorageTexturesPerShaderStage', {{{ C_STRUCTS.WGPULimits.maxStorageTexturesPerShaderStage }}});
      setLimitValueU32('maxUniformBuffersPerShaderStage', {{{ C_STRUCTS.WGPULimits.maxUniformBuffersPerShaderStage }}});
      setLimitValueU32('minUniformBufferOffsetAlignment', {{{ C_STRUCTS.WGPULimits.minUniformBufferOffsetAlignment }}});
      setLimitValueU32('minStorageBufferOffsetAlignment', {{{ C_STRUCTS.WGPULimits.minStorageBufferOffsetAlignment }}});

      setLimitValueU64('maxUniformBufferBindingSize', {{{ C_STRUCTS.WGPULimits.maxUniformBufferBindingSize }}});
      setLimitValueU64('maxStorageBufferBindingSize', {{{ C_STRUCTS.WGPULimits.maxStorageBufferBindingSize }}});

      setLimitValueU32('maxVertexBuffers', {{{ C_STRUCTS.WGPULimits.maxVertexBuffers }}});
      setLimitValueU64('maxBufferSize', {{{ C_STRUCTS.WGPULimits.maxBufferSize }}});
      setLimitValueU32('maxVertexAttributes', {{{ C_STRUCTS.WGPULimits.maxVertexAttributes }}});
      setLimitValueU32('maxVertexBufferArrayStride', {{{ C_STRUCTS.WGPULimits.maxVertexBufferArrayStride }}});
      setLimitValueU32('maxInterStageShaderVariables', {{{ C_STRUCTS.WGPULimits.maxInterStageShaderVariables }}});
      setLimitValueU32('maxColorAttachments', {{{ C_STRUCTS.WGPULimits.maxColorAttachments }}});
      setLimitValueU32('maxColorAttachmentBytesPerSample', {{{ C_STRUCTS.WGPULimits.maxColorAttachmentBytesPerSample }}});
      setLimitValueU32('maxComputeWorkgroupStorageSize', {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupStorageSize }}});
      setLimitValueU32('maxComputeInvocationsPerWorkgroup', {{{ C_STRUCTS.WGPULimits.maxComputeInvocationsPerWorkgroup }}});
      setLimitValueU32('maxComputeWorkgroupSizeX', {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupSizeX }}});
      setLimitValueU32('maxComputeWorkgroupSizeY', {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupSizeY }}});
      setLimitValueU32('maxComputeWorkgroupSizeZ', {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupSizeZ }}});
      setLimitValueU32('maxComputeWorkgroupsPerDimension', {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupsPerDimension }}});
    },

    // Maps from enum string back to enum number, for callbacks.
    {{{ WEBGPU_STRING_TO_INT_TABLES }}}

    // Maps from enum number to enum string.
    {{{ WEBGPU_INT_TO_STRING_TABLES }}}
  },

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
  emwgpuWaitAny__i53abi: false,
  emwgpuWaitAny__sig: 'jppp',
#if ASYNCIFY
  emwgpuWaitAny__async: true,
  emwgpuWaitAny: (futurePtr, futureCount, timeoutNSPtr) => Asyncify.handleAsync(async () => {
    var promises = [];
    if (timeoutNSPtr) {
      var timeoutMS = {{{ gpu.makeGetU64('timeoutNSPtr', 0) }}} / 1000000;
      promises.length = futureCount + 1;
      promises[futureCount] = new Promise((resolve) => setTimeout(resolve, timeoutMS, 0));
    } else {
      promises.length = futureCount;
    }

    for (var i = 0; i < futureCount; ++i) {
      // If any of the FutureIDs are not tracked, it means it must be done.
      var futureId = {{{ gpu.makeGetU64('(futurePtr + i * 8)', 0) }}};
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
  emwgpuWaitAny: () => {
    abort('TODO: Implement asyncify-free WaitAny for timeout=0');
  },
#endif

  emwgpuGetPreferredFormat__sig: 'i',
  emwgpuGetPreferredFormat: () => {
    var format = navigator["gpu"]["getPreferredCanvasFormat"]();
    return WebGPU.Int_PreferredFormat[format];
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

  wgpuGetInstanceCapabilities: (capabilitiesPtr) => {
    abort('TODO: wgpuGetInstanceCapabilities unimplemented');
    return 0;
  },

  wgpuGetProcAddress: (device, procName) => {
    abort('TODO(#11526): wgpuGetProcAddress unimplemented');
    return 0;
  },

  // --------------------------------------------------------------------------
  // Methods of Adapter
  // --------------------------------------------------------------------------

  wgpuAdapterGetFeatures__deps: ['malloc'],
  wgpuAdapterGetFeatures: (adapterPtr, supportedFeatures) => {
    var adapter = WebGPU.getJsObject(adapterPtr);

    // Always allocate enough space for all the features, though some may be unused.
    var featuresPtr = _malloc(adapter.features.size * 4);
    var offset = 0;
    var numFeatures = 0;
    adapter.features.forEach(feature => {
      var featureEnumValue = WebGPU.FeatureNameString2Enum[feature];
      if (featureEnumValue !== undefined) {
        {{{ makeSetValue('featuresPtr', 'offset', 'featureEnumValue', 'i32') }}};
        offset += 4;
        numFeatures++;
      }
    });
    {{{ makeSetValue('supportedFeatures', C_STRUCTS.WGPUSupportedFeatures.features, 'featuresPtr', '*') }}};
    {{{ makeSetValue('supportedFeatures', C_STRUCTS.WGPUSupportedFeatures.featureCount, 'numFeatures', '*') }}};
  },

  wgpuAdapterGetInfo__deps: ['$stringToNewUTF8', '$lengthBytesUTF8'],
  wgpuAdapterGetInfo: (adapterPtr, info) => {
    var adapter = WebGPU.getJsObject(adapterPtr);
    {{{ gpu.makeCheckDescriptor('info') }}}

    // Append all the strings together to condense into a single malloc.
    var strs = adapter.info.vendor + adapter.info.architecture + adapter.info.device + adapter.info.description;
    var strPtr = stringToNewUTF8(strs);

    var vendorLen = lengthBytesUTF8(adapter.info.vendor);
    WebGPU.setStringView(info + {{{ C_STRUCTS.WGPUAdapterInfo.vendor }}}, strPtr, vendorLen);
    strPtr += vendorLen;

    var architectureLen = lengthBytesUTF8(adapter.info.architecture);
    WebGPU.setStringView(info + {{{ C_STRUCTS.WGPUAdapterInfo.architecture }}}, strPtr, architectureLen);
    strPtr += architectureLen;

    var deviceLen = lengthBytesUTF8(adapter.info.device);
    WebGPU.setStringView(info + {{{ C_STRUCTS.WGPUAdapterInfo.device }}}, strPtr, deviceLen);
    strPtr += deviceLen;

    var descriptionLen = lengthBytesUTF8(adapter.info.description);
    WebGPU.setStringView(info + {{{ C_STRUCTS.WGPUAdapterInfo.description }}}, strPtr, descriptionLen);
    strPtr += descriptionLen;

    {{{ makeSetValue('info', C_STRUCTS.WGPUAdapterInfo.backendType, gpu.BackendType.WebGPU, 'i32') }}};
    var adapterType = adapter.info.isFallbackAdapter ? {{{ gpu.AdapterType.CPU }}} : {{{ gpu.AdapterType.Unknown }}};
    {{{ makeSetValue('info', C_STRUCTS.WGPUAdapterInfo.adapterType, 'adapterType', 'i32') }}};
    {{{ makeSetValue('info', C_STRUCTS.WGPUAdapterInfo.vendorID, '0', 'i32') }}};
    {{{ makeSetValue('info', C_STRUCTS.WGPUAdapterInfo.deviceID, '0', 'i32') }}};
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

  emwgpuAdapterRequestDevice__deps: ['emwgpuOnDeviceLostCompleted', 'emwgpuOnRequestDeviceCompleted', 'emwgpuOnUncapturedError'],
  emwgpuAdapterRequestDevice__sig: 'vpjjppp',
  emwgpuAdapterRequestDevice: (adapterPtr, futureId, deviceLostFutureId, devicePtr, queuePtr, descriptor) => {
    var adapter = WebGPU.getJsObject(adapterPtr);

    var desc = {};
    if (descriptor) {
      {{{ gpu.makeCheckDescriptor('descriptor') }}}
      var requiredFeatureCount = {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUDeviceDescriptor.requiredFeatureCount) }}};
      if (requiredFeatureCount) {
        var requiredFeaturesPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUDeviceDescriptor.requiredFeatures, '*') }}};
        // requiredFeaturesPtr is a pointer to an array of FeatureName which is an enum of size uint32_t
        desc["requiredFeatures"] = Array.from({{{ makeHEAPView('U32', 'requiredFeaturesPtr', `requiredFeaturesPtr + requiredFeatureCount * 4`) }}},
          (feature) => WebGPU.FeatureName[feature]);
      }
      var limitsPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPUDeviceDescriptor.requiredLimits, '*') }}};
      if (limitsPtr) {
        {{{ gpu.makeCheckDescriptor('limitsPtr') }}}
        var requiredLimits = {};
        function setLimitU32IfDefined(name, limitOffset) {
          var ptr = limitsPtr + limitOffset;
          var value = {{{ gpu.makeGetU32('ptr', 0) }}};
          if (value != {{{ gpu.LIMIT_U32_UNDEFINED }}}) {
            requiredLimits[name] = value;
          }
        }
        function setLimitU64IfDefined(name, limitOffset) {
          var ptr = limitsPtr + limitOffset;
          // Handle WGPU_LIMIT_U64_UNDEFINED.
          var limitPart1 = {{{ gpu.makeGetU32('ptr', 0) }}};
          var limitPart2 = {{{ gpu.makeGetU32('ptr', 4) }}};
          if (limitPart1 != 0xFFFFFFFF || limitPart2 != 0xFFFFFFFF) {
            requiredLimits[name] = {{{ gpu.makeGetU64('ptr', 0) }}}
          }
        }

        setLimitU32IfDefined("maxTextureDimension1D", {{{ C_STRUCTS.WGPULimits.maxTextureDimension1D }}});
        setLimitU32IfDefined("maxTextureDimension2D", {{{ C_STRUCTS.WGPULimits.maxTextureDimension2D }}});
        setLimitU32IfDefined("maxTextureDimension3D", {{{ C_STRUCTS.WGPULimits.maxTextureDimension3D }}});
        setLimitU32IfDefined("maxTextureArrayLayers", {{{ C_STRUCTS.WGPULimits.maxTextureArrayLayers }}});
        setLimitU32IfDefined("maxBindGroups", {{{ C_STRUCTS.WGPULimits.maxBindGroups }}});
        setLimitU32IfDefined('maxBindGroupsPlusVertexBuffers', {{{ C_STRUCTS.WGPULimits.maxBindGroupsPlusVertexBuffers }}});
        setLimitU32IfDefined("maxDynamicUniformBuffersPerPipelineLayout", {{{ C_STRUCTS.WGPULimits.maxDynamicUniformBuffersPerPipelineLayout }}});
        setLimitU32IfDefined("maxDynamicStorageBuffersPerPipelineLayout", {{{ C_STRUCTS.WGPULimits.maxDynamicStorageBuffersPerPipelineLayout }}});
        setLimitU32IfDefined("maxSampledTexturesPerShaderStage", {{{ C_STRUCTS.WGPULimits.maxSampledTexturesPerShaderStage }}});
        setLimitU32IfDefined("maxSamplersPerShaderStage", {{{ C_STRUCTS.WGPULimits.maxSamplersPerShaderStage }}});
        setLimitU32IfDefined("maxStorageBuffersPerShaderStage", {{{ C_STRUCTS.WGPULimits.maxStorageBuffersPerShaderStage }}});
        setLimitU32IfDefined("maxStorageTexturesPerShaderStage", {{{ C_STRUCTS.WGPULimits.maxStorageTexturesPerShaderStage }}});
        setLimitU32IfDefined("maxUniformBuffersPerShaderStage", {{{ C_STRUCTS.WGPULimits.maxUniformBuffersPerShaderStage }}});
        setLimitU32IfDefined("minUniformBufferOffsetAlignment", {{{ C_STRUCTS.WGPULimits.minUniformBufferOffsetAlignment }}});
        setLimitU32IfDefined("minStorageBufferOffsetAlignment", {{{ C_STRUCTS.WGPULimits.minStorageBufferOffsetAlignment }}});
        setLimitU64IfDefined("maxUniformBufferBindingSize", {{{ C_STRUCTS.WGPULimits.maxUniformBufferBindingSize }}});
        setLimitU64IfDefined("maxStorageBufferBindingSize", {{{ C_STRUCTS.WGPULimits.maxStorageBufferBindingSize }}});
        setLimitU32IfDefined("maxVertexBuffers", {{{ C_STRUCTS.WGPULimits.maxVertexBuffers }}});
        setLimitU64IfDefined("maxBufferSize", {{{ C_STRUCTS.WGPULimits.maxBufferSize }}});
        setLimitU32IfDefined("maxVertexAttributes", {{{ C_STRUCTS.WGPULimits.maxVertexAttributes }}});
        setLimitU32IfDefined("maxVertexBufferArrayStride", {{{ C_STRUCTS.WGPULimits.maxVertexBufferArrayStride }}});
        setLimitU32IfDefined("maxInterStageShaderVariables", {{{ C_STRUCTS.WGPULimits.maxInterStageShaderVariables }}});
        setLimitU32IfDefined("maxColorAttachments", {{{ C_STRUCTS.WGPULimits.maxColorAttachments }}});
        setLimitU32IfDefined("maxColorAttachmentBytesPerSample", {{{ C_STRUCTS.WGPULimits.maxColorAttachmentBytesPerSample }}});
        setLimitU32IfDefined("maxComputeWorkgroupStorageSize", {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupStorageSize }}});
        setLimitU32IfDefined("maxComputeInvocationsPerWorkgroup", {{{ C_STRUCTS.WGPULimits.maxComputeInvocationsPerWorkgroup }}});
        setLimitU32IfDefined("maxComputeWorkgroupSizeX", {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupSizeX }}});
        setLimitU32IfDefined("maxComputeWorkgroupSizeY", {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupSizeY }}});
        setLimitU32IfDefined("maxComputeWorkgroupSizeZ", {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupSizeZ }}});
        setLimitU32IfDefined("maxComputeWorkgroupsPerDimension", {{{ C_STRUCTS.WGPULimits.maxComputeWorkgroupsPerDimension }}});
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

    {{{ runtimeKeepalivePush() }}}
    WebGPU.Internals.futureInsert(futureId, adapter.requestDevice(desc).then((device) => {
      {{{ runtimeKeepalivePop() }}}
      WebGPU.Internals.jsObjectInsert(queuePtr, device.queue);
      WebGPU.Internals.jsObjectInsert(devicePtr, device);

      {{{ gpu.convertToPassAsPointer('devicePtr') }}}

      // Set up device lost promise resolution.
      if (deviceLostFutureId) {
        {{{ runtimeKeepalivePush() }}}
        WebGPU.Internals.futureInsert(deviceLostFutureId, device.lost.then((info) => {
          {{{ runtimeKeepalivePop() }}}
          // Unset the uncaptured error handler.
          device.onuncapturederror = (ev) => {};
          var sp = stackSave();
          var messagePtr = stringToUTF8OnStack(info.message);
          _emwgpuOnDeviceLostCompleted(deviceLostFutureId, WebGPU.Int_DeviceLostReason[info.reason],
            {{{ gpu.passAsPointer('messagePtr') }}});
          stackRestore(sp);
        }));
      }

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
    }, (ex) => {
      {{{ runtimeKeepalivePop() }}}
      var sp = stackSave();
      var messagePtr = stringToUTF8OnStack(ex.message);
      _emwgpuOnRequestDeviceCompleted(futureId, {{{ gpu.RequestDeviceStatus.Error }}},
        {{{ gpu.passAsPointer('devicePtr') }}}, {{{ gpu.passAsPointer('messagePtr') }}});
      if (deviceLostFutureId) {
        _emwgpuOnDeviceLostCompleted(deviceLostFutureId, {{{ gpu.DeviceLostReason.FailedCreation }}},
          {{{ gpu.passAsPointer('messagePtr') }}});
      }
      stackRestore(sp);
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

    if (size === 0) warnOnce('getMappedRange size=0 no longer means WGPU_WHOLE_MAP_SIZE');

    {{{ gpu.convertSentinelToUndefined('size', true) }}}

    var mapped;
    try {
      mapped = buffer.getMappedRange(offset, size);
    } catch (ex) {
#if ASSERTIONS
      err(`buffer.getMappedRange(${offset}, ${size}) failed: ${ex}`);
#endif
      return 0;
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

    if (size === 0) warnOnce('getMappedRange size=0 no longer means WGPU_WHOLE_MAP_SIZE');

    {{{ gpu.convertSentinelToUndefined('size', true) }}}

    var mapped;
    try {
      mapped = buffer.getMappedRange(offset, size);
    } catch (ex) {
#if ASSERTIONS
      err(`buffer.getMappedRange(${offset}, ${size}) failed: ${ex}`);
#endif
      return 0;
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
  emwgpuBufferMapAsync__deps: ['emwgpuOnMapAsyncCompleted'],
  emwgpuBufferMapAsync__sig: 'vpjjpp',
  emwgpuBufferMapAsync: (bufferPtr, futureId, mode, offset, size) => {
    var buffer = WebGPU.getJsObject(bufferPtr);
    WebGPU.Internals.bufferOnUnmaps[bufferPtr] = [];

    {{{ gpu.convertSentinelToUndefined('size', true) }}}

    {{{ runtimeKeepalivePush() }}}
    WebGPU.Internals.futureInsert(futureId, buffer.mapAsync(mode, offset, size).then(() => {
      {{{ runtimeKeepalivePop() }}}
      _emwgpuOnMapAsyncCompleted(futureId, {{{ gpu.MapAsyncStatus.Success }}},
        {{{ gpu.NULLPTR }}});
    }, (ex) => {
      {{{ runtimeKeepalivePop() }}}
      var sp = stackSave();
      var messagePtr = stringToUTF8OnStack(ex.message);
      var status =
        ex.name === 'AbortError' ? {{{ gpu.MapAsyncStatus.Aborted }}} :
        ex.name === 'OperationError' ? {{{ gpu.MapAsyncStatus.Error }}} :
        0;
      {{{ gpu.makeCheck('status') }}}
      _emwgpuOnMapAsyncCompleted(futureId, status, {{{ gpu.passAsPointer('messagePtr') }}});
      delete WebGPU.Internals.bufferOnUnmaps[bufferPtr];
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
      var viewPtr = {{{ gpu.makeGetU32('caPtr', C_STRUCTS.WGPURenderPassColorAttachment.view) }}};
      if (viewPtr === 0) {
        // view could be undefined.
        return undefined;
      }

      var depthSlice = {{{ makeGetValue('caPtr', C_STRUCTS.WGPURenderPassColorAttachment.depthSlice, 'i32') }}};
      {{{ gpu.convertSentinelToUndefined('depthSlice') }}}

      var loadOpInt = {{{ gpu.makeGetU32('caPtr', C_STRUCTS.WGPURenderPassColorAttachment.loadOp) }}};
      #if ASSERTIONS
          assert(loadOpInt !== {{{ gpu.LoadOp.Undefined }}});
      #endif

      var storeOpInt = {{{ gpu.makeGetU32('caPtr', C_STRUCTS.WGPURenderPassColorAttachment.storeOp) }}};
      #if ASSERTIONS
          assert(storeOpInt !== {{{ gpu.StoreOp.Undefined }}});
      #endif

      var clearValue = WebGPU.makeColor(caPtr + {{{ C_STRUCTS.WGPURenderPassColorAttachment.clearValue }}});

      return {
        "view": WebGPU.getJsObject(viewPtr),
        "depthSlice": depthSlice,
        "resolveTarget": WebGPU.getJsObject(
          {{{ gpu.makeGetU32('caPtr', C_STRUCTS.WGPURenderPassColorAttachment.resolveTarget) }}}),
        "clearValue": clearValue,
        "loadOp":  WebGPU.LoadOp[loadOpInt],
        "storeOp": WebGPU.StoreOp[storeOpInt],
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
          {{{ gpu.makeGetU32('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.view) }}}),
        "depthClearValue": {{{ makeGetValue('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.depthClearValue, 'float') }}},
        "depthLoadOp": WebGPU.LoadOp[
          {{{ gpu.makeGetU32('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.depthLoadOp) }}}],
        "depthStoreOp": WebGPU.StoreOp[
          {{{ gpu.makeGetU32('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.depthStoreOp) }}}],
        "depthReadOnly": {{{ gpu.makeGetBool('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.depthReadOnly) }}},
        "stencilClearValue": {{{ gpu.makeGetU32('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.stencilClearValue) }}},
        "stencilLoadOp": WebGPU.LoadOp[
          {{{ gpu.makeGetU32('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.stencilLoadOp) }}}],
        "stencilStoreOp": WebGPU.StoreOp[
          {{{ gpu.makeGetU32('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.stencilStoreOp) }}}],
        "stencilReadOnly": {{{ gpu.makeGetBool('dsaPtr', C_STRUCTS.WGPURenderPassDepthStencilAttachment.stencilReadOnly) }}},
      };
    }

    function makeRenderPassDescriptor(descriptor) {
      {{{ gpu.makeCheck('descriptor') }}}
      var nextInChainPtr = {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderPassDescriptor.nextInChain, '*') }}};

      var maxDrawCount = undefined;
      if (nextInChainPtr !== 0) {
        var sType = {{{ gpu.makeGetU32('nextInChainPtr', C_STRUCTS.WGPUChainedStruct.sType) }}};
#if ASSERTIONS
        assert(sType === {{{ gpu.SType.RenderPassMaxDrawCount }}});
        assert(0 === {{{ makeGetValue('nextInChainPtr', gpu.kOffsetOfNextInChainMember, '*') }}});
#endif
        var renderPassMaxDrawCount = nextInChainPtr;
        {{{ gpu.makeCheckDescriptor('renderPassMaxDrawCount') }}}
        maxDrawCount = {{{ gpu.makeGetU64('renderPassMaxDrawCount', C_STRUCTS.WGPURenderPassMaxDrawCount.maxDrawCount) }}};
      }

      var desc = {
        "label": WebGPU.makeStringFromOptionalStringView(
          descriptor + {{{ C_STRUCTS.WGPURenderPassDescriptor.label }}}),
        "colorAttachments": makeColorAttachments(
          {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPURenderPassDescriptor.colorAttachmentCount) }}},
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
    {{{ gpu.convertSentinelToUndefined('size') }}}

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

  wgpuCommandEncoderInsertDebugMarker2: (encoderPtr, markerLabelPtr) => {
    var encoder = WebGPU.getJsObject(encoderPtr);
    encoder.insertDebugMarker(WebGPU.makeStringFromStringView(markerLabelPtr));
  },

  wgpuCommandEncoderPopDebugGroup: (encoderPtr) => {
    var encoder = WebGPU.getJsObject(encoderPtr);
    encoder.popDebugGroup();
  },

  wgpuCommandEncoderPushDebugGroup2: (encoderPtr, groupLabelPtr) => {
    var encoder = WebGPU.getJsObject(encoderPtr);
    encoder.pushDebugGroup(WebGPU.makeStringFromStringView(groupLabelPtr));
  },

  wgpuCommandEncoderResolveQuerySet: (encoderPtr, querySetPtr, firstQuery, queryCount, destinationPtr, destinationOffset) => {
    var commandEncoder = WebGPU.getJsObject(encoderPtr);
    var querySet = WebGPU.getJsObject(querySetPtr);
    var destination = WebGPU.getJsObject(destinationPtr);

    commandEncoder.resolveQuerySet(querySet, firstQuery, queryCount, destination, destinationOffset);
  },

  wgpuCommandEncoderWriteTimestamp: (encoderPtr, querySetPtr, queryIndex) => {
    var commandEncoder = WebGPU.getJsObject(encoderPtr);
    var querySet = WebGPU.getJsObject(querySetPtr);
    commandEncoder.writeTimestamp(querySet, queryIndex);
  },

  // --------------------------------------------------------------------------
  // Methods of ComputePassEncoder
  // --------------------------------------------------------------------------

  wgpuComputePassEncoderDispatchWorkgroups: (passPtr, x, y, z) => {
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

  wgpuComputePassEncoderInsertDebugMarker2: (encoderPtr, markerLabelPtr) => {
    var encoder = WebGPU.getJsObject(encoderPtr);
    encoder.insertDebugMarker(WebGPU.makeStringFromStringView(markerLabelPtr));
  },

  wgpuComputePassEncoderPopDebugGroup: (encoderPtr) => {
    var encoder = WebGPU.getJsObject(encoderPtr);
    encoder.popDebugGroup();
  },

  wgpuComputePassEncoderPushDebugGroup2: (encoderPtr, groupLabelPtr) => {
    var encoder = WebGPU.getJsObject(encoderPtr);
    encoder.pushDebugGroup(WebGPU.makeStringFromStringView(groupLabelPtr));
  },

  wgpuComputePassEncoderSetBindGroup: (passPtr, groupIndex, groupPtr, dynamicOffsetCount, dynamicOffsetsPtr) => {
    var pass = WebGPU.getJsObject(passPtr);
    var group = WebGPU.getJsObject(groupPtr);
    if (dynamicOffsetCount == 0) {
      pass.setBindGroup(groupIndex, group);
    } else {
      var offsets = [];
      for (var i = 0; i < dynamicOffsetCount; i++, dynamicOffsetsPtr += 4) {
        offsets.push({{{ gpu.makeGetU32('dynamicOffsetsPtr', 0) }}});
      }
      pass.setBindGroup(groupIndex, group, offsets);
    }
  },

  wgpuComputePassEncoderSetPipeline: (passPtr, pipelinePtr) => {
    var pass = WebGPU.getJsObject(passPtr);
    var pipeline = WebGPU.getJsObject(pipelinePtr);
    pass.setPipeline(pipeline);
  },

  wgpuComputePassEncoderWriteTimestamp: (encoderPtr, querySetPtr, queryIndex) => {
    var encoder = WebGPU.getJsObject(encoderPtr);
    var querySet = WebGPU.getJsObject(querySetPtr);
    encoder.writeTimestamp(querySet, queryIndex);
  },

  // --------------------------------------------------------------------------
  // Methods of ComputePipeline
  // --------------------------------------------------------------------------

  wgpuComputePipelineGetBindGroupLayout__deps: ['emwgpuCreateBindGroupLayout'],
  wgpuComputePipelineGetBindGroupLayout: (pipelinePtr, groupIndex) => {
    var pipeline = WebGPU.getJsObject(pipelinePtr);
    var ptr = _emwgpuCreateBindGroupLayout({{{ gpu.NULLPTR }}});
    WebGPU.Internals.jsObjectInsert(ptr, pipeline.getBindGroupLayout(groupIndex));
    return ptr;
  },

  // --------------------------------------------------------------------------
  // Methods of Device
  // --------------------------------------------------------------------------

  wgpuDeviceCreateBindGroup__deps: ['$readI53FromI64', 'emwgpuCreateBindGroup'],
  wgpuDeviceCreateBindGroup: (devicePtr, descriptor) => {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}

    function makeEntry(entryPtr) {
      {{{ gpu.makeCheck('entryPtr') }}}

      var bufferPtr = {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupEntry.buffer) }}};
      var samplerPtr = {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupEntry.sampler) }}};
      var textureViewPtr = {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupEntry.textureView) }}};
#if ASSERTIONS
      assert((bufferPtr !== 0) + (samplerPtr !== 0) + (textureViewPtr !== 0) === 1);
#endif

      var binding = {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupEntry.binding) }}};

      if (bufferPtr) {
        var size = {{{ makeGetValue('entryPtr', C_STRUCTS.WGPUBindGroupEntry.size, 'i53') }}};
        {{{ gpu.convertSentinelToUndefined('size') }}}

        return {
          "binding": binding,
          "resource": {
            "buffer": WebGPU.getJsObject(bufferPtr),
            "offset": {{{ gpu.makeGetU64('entryPtr', C_STRUCTS.WGPUBindGroupEntry.offset) }}},
            "size": size
          },
        };
      } else if (samplerPtr) {
        return {
          "binding": binding,
          "resource": WebGPU.getJsObject(samplerPtr),
        };
      } else {
        return {
          "binding": binding,
          "resource": WebGPU.getJsObject(textureViewPtr),
        };
      }
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
        {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUBindGroupDescriptor.entryCount) }}},
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

    function makeBufferEntry(entryPtr) {
      {{{ gpu.makeCheck('entryPtr') }}}

      var typeInt =
        {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBufferBindingLayout.type) }}};
      if (!typeInt) return undefined;

      return {
        "type": WebGPU.BufferBindingType[typeInt],
        "hasDynamicOffset":
          {{{ gpu.makeGetBool('entryPtr', C_STRUCTS.WGPUBufferBindingLayout.hasDynamicOffset) }}},
        "minBindingSize":
          {{{ gpu.makeGetU64('entryPtr', C_STRUCTS.WGPUBufferBindingLayout.minBindingSize) }}},
      };
    }

    function makeSamplerEntry(entryPtr) {
      {{{ gpu.makeCheck('entryPtr') }}}

      var typeInt =
        {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUSamplerBindingLayout.type) }}};
      if (!typeInt) return undefined;

      return {
        "type": WebGPU.SamplerBindingType[typeInt],
      };
    }

    function makeTextureEntry(entryPtr) {
      {{{ gpu.makeCheck('entryPtr') }}}

      var sampleTypeInt =
        {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUTextureBindingLayout.sampleType) }}};
      if (!sampleTypeInt) return undefined;

      return {
        "sampleType": WebGPU.TextureSampleType[sampleTypeInt],
        "viewDimension": WebGPU.TextureViewDimension[
          {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUTextureBindingLayout.viewDimension) }}}],
        "multisampled":
          {{{ gpu.makeGetBool('entryPtr', C_STRUCTS.WGPUTextureBindingLayout.multisampled) }}},
      };
    }

    function makeStorageTextureEntry(entryPtr) {
      {{{ gpu.makeCheck('entryPtr') }}}

      var accessInt =
        {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUStorageTextureBindingLayout.access) }}}
      if (!accessInt) return undefined;

      return {
        "access": WebGPU.StorageTextureAccess[accessInt],
        "format": WebGPU.TextureFormat[
          {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUStorageTextureBindingLayout.format) }}}],
        "viewDimension": WebGPU.TextureViewDimension[
          {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUStorageTextureBindingLayout.viewDimension) }}}],
      };
    }

    function makeEntry(entryPtr) {
      {{{ gpu.makeCheck('entryPtr') }}}

      return {
        "binding":
          {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupLayoutEntry.binding) }}},
        "visibility":
          {{{ gpu.makeGetU32('entryPtr', C_STRUCTS.WGPUBindGroupLayoutEntry.visibility) }}},
        "buffer": makeBufferEntry(entryPtr + {{{ C_STRUCTS.WGPUBindGroupLayoutEntry.buffer }}}),
        "sampler": makeSamplerEntry(entryPtr + {{{ C_STRUCTS.WGPUBindGroupLayoutEntry.sampler }}}),
        "texture": makeTextureEntry(entryPtr + {{{ C_STRUCTS.WGPUBindGroupLayoutEntry.texture }}}),
        "storageTexture": makeStorageTextureEntry(entryPtr + {{{ C_STRUCTS.WGPUBindGroupLayoutEntry.storageTexture }}}),
      };
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
        {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUBindGroupLayoutDescriptor.entryCount) }}},
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
      "usage": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUBufferDescriptor.usage) }}},
      "size": {{{ gpu.makeGetU64('descriptor', C_STRUCTS.WGPUBufferDescriptor.size) }}},
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

  emwgpuDeviceCreateComputePipelineAsync__deps: ['emwgpuOnCreateComputePipelineCompleted'],
  emwgpuDeviceCreateComputePipelineAsync__sig: 'vpjpp',
  emwgpuDeviceCreateComputePipelineAsync: (devicePtr, futureId, descriptor, pipelinePtr) => {
    var desc = WebGPU.makeComputePipelineDesc(descriptor);
    var device = WebGPU.getJsObject(devicePtr);
    {{{ runtimeKeepalivePush() }}}
    WebGPU.Internals.futureInsert(futureId, device.createComputePipelineAsync(desc).then((pipeline) => {
      {{{ runtimeKeepalivePop() }}}
      WebGPU.Internals.jsObjectInsert(pipelinePtr, pipeline);
      _emwgpuOnCreateComputePipelineCompleted(futureId, {{{ gpu.CreatePipelineAsyncStatus.Success }}},
        {{{ gpu.passAsPointer('pipelinePtr') }}}, {{{ gpu.NULLPTR }}});
    }, (pipelineError) => {
      {{{ runtimeKeepalivePop() }}}
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
    }));
  },

  wgpuDeviceCreatePipelineLayout__deps: ['emwgpuCreatePipelineLayout'],
  wgpuDeviceCreatePipelineLayout: (devicePtr, descriptor) => {
    {{{ gpu.makeCheckDescriptor('descriptor') }}}
    var bglCount = {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUPipelineLayoutDescriptor.bindGroupLayoutCount) }}};
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
      "type": WebGPU.QueryType[
        {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUQuerySetDescriptor.type) }}}],
      "count": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUQuerySetDescriptor.count) }}},
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
          // format could be undefined
          formats.push(WebGPU.TextureFormat[{{{ gpu.makeGetU32('formatsPtr', 0) }}}]);
        }
        return formats;
      }

      var desc = {
        "label": WebGPU.makeStringFromOptionalStringView(
          descriptor + {{{ C_STRUCTS.WGPURenderBundleEncoderDescriptor.label }}}),
        "colorFormats": makeColorFormats(
          {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPURenderBundleEncoderDescriptor.colorFormatCount) }}},
          {{{ makeGetValue('descriptor', C_STRUCTS.WGPURenderBundleEncoderDescriptor.colorFormats, '*') }}}),
        "depthStencilFormat": WebGPU.TextureFormat[{{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPURenderBundleEncoderDescriptor.depthStencilFormat) }}}],
        "sampleCount": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPURenderBundleEncoderDescriptor.sampleCount) }}},
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

  emwgpuDeviceCreateRenderPipelineAsync__deps: ['emwgpuOnCreateRenderPipelineCompleted'],
  emwgpuDeviceCreateRenderPipelineAsync__sig: 'vpjpp',
  emwgpuDeviceCreateRenderPipelineAsync: (devicePtr, futureId, descriptor, pipelinePtr) => {
    var desc = WebGPU.makeRenderPipelineDesc(descriptor);
    var device = WebGPU.getJsObject(devicePtr);
    {{{ runtimeKeepalivePush() }}}
    WebGPU.Internals.futureInsert(futureId, device.createRenderPipelineAsync(desc).then((pipeline) => {
      {{{ runtimeKeepalivePop() }}}
      WebGPU.Internals.jsObjectInsert(pipelinePtr, pipeline);
      _emwgpuOnCreateRenderPipelineCompleted(futureId, {{{ gpu.CreatePipelineAsyncStatus.Success }}},
        {{{ gpu.passAsPointer('pipelinePtr') }}}, {{{ gpu.NULLPTR }}});
    }, (pipelineError) => {
      {{{ runtimeKeepalivePop() }}}
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
        "addressModeU": WebGPU.AddressMode[
            {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSamplerDescriptor.addressModeU) }}}],
        "addressModeV": WebGPU.AddressMode[
            {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSamplerDescriptor.addressModeV) }}}],
        "addressModeW": WebGPU.AddressMode[
            {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSamplerDescriptor.addressModeW) }}}],
        "magFilter": WebGPU.FilterMode[
            {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSamplerDescriptor.magFilter) }}}],
        "minFilter": WebGPU.FilterMode[
            {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSamplerDescriptor.minFilter) }}}],
        "mipmapFilter": WebGPU.MipmapFilterMode[
            {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSamplerDescriptor.mipmapFilter) }}}],
        "lodMinClamp": {{{ makeGetValue('descriptor', C_STRUCTS.WGPUSamplerDescriptor.lodMinClamp, 'float') }}},
        "lodMaxClamp": {{{ makeGetValue('descriptor', C_STRUCTS.WGPUSamplerDescriptor.lodMaxClamp, 'float') }}},
        "compare": WebGPU.CompareFunction[
            {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUSamplerDescriptor.compare) }}}],
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
    var sType = {{{ gpu.makeGetU32('nextInChainPtr', C_STRUCTS.WGPUChainedStruct.sType) }}};

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
    {{{ gpu.makeCheckDescriptor('descriptor') }}}

    var desc = {
      "label": WebGPU.makeStringFromOptionalStringView(
        descriptor + {{{ C_STRUCTS.WGPUTextureDescriptor.label }}}),
      "size": WebGPU.makeExtent3D(descriptor + {{{ C_STRUCTS.WGPUTextureDescriptor.size }}}),
      "mipLevelCount": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureDescriptor.mipLevelCount) }}},
      "sampleCount": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureDescriptor.sampleCount) }}},
      "dimension": WebGPU.TextureDimension[
        {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureDescriptor.dimension) }}}],
      "format": WebGPU.TextureFormat[
        {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureDescriptor.format) }}}],
      "usage": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureDescriptor.usage) }}},
    };

    var viewFormatCount = {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureDescriptor.viewFormatCount) }}};
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
    WebGPU.getJsObject(devicePtr).destroy()
  },

  wgpuDeviceGetFeatures__deps: ['malloc'],
  wgpuDeviceGetFeatures: (devicePtr, supportedFeatures) => {
    var device = WebGPU.getJsObject(devicePtr);

    // Always allocate enough space for all the features, though some may be unused.
    var featuresPtr = _malloc(device.features.size * 4);
    var offset = 0;
    var numFeatures = 0;
    device.features.forEach(feature => {
      var featureEnumValue = WebGPU.FeatureNameString2Enum[feature];
      if (featureEnumValue !== undefined) {
        {{{ makeSetValue('featuresPtr', 'offset', 'featureEnumValue', 'i32') }}};
        offset += 4;
        numFeatures++;
      }
    });
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

  wgpuDeviceGetAdapterInfo__deps: ['$stringToNewUTF8', '$lengthBytesUTF8'],
  wgpuDeviceGetAdapterInfo: (devicePtr, adapterInfo) => {
    // TODO(crbug.com/377760848): Avoid duplicated code with wgpuAdapterGetInfo,
    // for example by deferring to wgpuAdapterGetInfo from webgpu.cpp.
    var device = WebGPU.getJsObject(devicePtr);
    {{{ gpu.makeCheckDescriptor('adapterInfo') }}}

    // Append all the strings together to condense into a single malloc.
    var strs = device.adapterInfo.vendor + device.adapterInfo.architecture + device.adapterInfo.device + device.adapterInfo.description;
    var strPtr = stringToNewUTF8(strs);

    var vendorLen = lengthBytesUTF8(device.adapterInfo.vendor);
    WebGPU.setStringView(adapterInfo + {{{ C_STRUCTS.WGPUAdapterInfo.vendor }}}, strPtr, vendorLen);
    strPtr += vendorLen;

    var architectureLen = lengthBytesUTF8(device.adapterInfo.architecture);
    WebGPU.setStringView(adapterInfo + {{{ C_STRUCTS.WGPUAdapterInfo.architecture }}}, strPtr, architectureLen);
    strPtr += architectureLen;

    var deviceLen = lengthBytesUTF8(device.adapterInfo.device);
    WebGPU.setStringView(adapterInfo + {{{ C_STRUCTS.WGPUAdapterInfo.device }}}, strPtr, deviceLen);
    strPtr += deviceLen;

    var descriptionLen = lengthBytesUTF8(device.adapterInfo.description);
    WebGPU.setStringView(adapterInfo + {{{ C_STRUCTS.WGPUAdapterInfo.description }}}, strPtr, descriptionLen);
    strPtr += descriptionLen;

    {{{ makeSetValue('adapterInfo', C_STRUCTS.WGPUAdapterInfo.backendType, gpu.BackendType.WebGPU, 'i32') }}};
    var adapterType = device.adapterInfo.isFallbackAdapter ? {{{ gpu.AdapterType.CPU }}} : {{{ gpu.AdapterType.Unknown }}};
    {{{ makeSetValue('adapterInfo', C_STRUCTS.WGPUAdapterInfo.adapterType, 'adapterType', 'i32') }}};
    {{{ makeSetValue('adapterInfo', C_STRUCTS.WGPUAdapterInfo.vendorID, '0', 'i32') }}};
    {{{ makeSetValue('adapterInfo', C_STRUCTS.WGPUAdapterInfo.deviceID, '0', 'i32') }}};
    return {{{ gpu.Status.Success }}};
  },

  emwgpuDevicePopErrorScope__deps: ['emwgpuOnPopErrorScopeCompleted'],
  emwgpuDevicePopErrorScope__sig: 'vpj',
  emwgpuDevicePopErrorScope: (devicePtr, futureId) => {
    var device = WebGPU.getJsObject(devicePtr);
    {{{ runtimeKeepalivePush() }}}
    WebGPU.Internals.futureInsert(futureId, device.popErrorScope().then((gpuError) => {
      {{{ runtimeKeepalivePop() }}}
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
    }, (ex) => {
      {{{ runtimeKeepalivePop() }}}
      var sp = stackSave();
      var messagePtr = stringToUTF8OnStack(ex.message);
      _emwgpuOnPopErrorScopeCompleted(futureId,
        {{{ gpu.PopErrorScopeStatus.Success }}}, {{{ gpu.ErrorType.Unknown }}},
        {{{ gpu.passAsPointer('messagePtr') }}});
      stackRestore(sp);
    }));
  },

  wgpuDevicePushErrorScope: (devicePtr, filter) => {
    var device = WebGPU.getJsObject(devicePtr);
    device.pushErrorScope(WebGPU.ErrorFilter[filter]);
  },

  // TODO(crbug.com/42241415): Remove this after verifying that it's not used and/or updating users.
  wgpuDeviceSetUncapturedErrorCallback__deps: ['$callUserCallback'],
  wgpuDeviceSetUncapturedErrorCallback: (devicePtr, callback, userdata) => {
    var device = WebGPU.getJsObject(devicePtr);
    device.onuncapturederror = function(ev) {
      // This will skip the callback if the runtime is no longer alive.
      callUserCallback(() => {
        // WGPUErrorType type, const char* message, void* userdata
        var Validation = 0x00000001;
        var OutOfMemory = 0x00000002;
        var type;
#if ASSERTIONS
        assert(typeof GPUValidationError != 'undefined');
        assert(typeof GPUOutOfMemoryError != 'undefined');
#endif
        if (ev.error instanceof GPUValidationError) type = Validation;
        else if (ev.error instanceof GPUOutOfMemoryError) type = OutOfMemory;
        // TODO: Implement GPUInternalError

        WebGPU.errorCallback(callback, type, ev.error.message, userdata);
      });
    };
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
      {{{ gpu.makeGetU32('nextInChainPtr', C_STRUCTS.WGPUChainedStruct.sType) }}});
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
    if (!context) return 0;

    context.surfaceLabelWebGPU = WebGPU.makeStringFromOptionalStringView(
      descriptor + {{{ C_STRUCTS.WGPUSurfaceDescriptor.label }}}
    );

    var ptr = _emwgpuCreateSurface({{{ gpu.NULLPTR }}});
    WebGPU.Internals.jsObjectInsert(ptr, context);
    return ptr;
  },

  wgpuInstanceHasWGSLLanguageFeature: (instance, featureEnumValue) => {
    if (!('wgslLanguageFeatures' in navigator["gpu"])) {
      return false;
    }
    return navigator["gpu"]["wgslLanguageFeatures"].has(WebGPU.WGSLLanguageFeatureName[featureEnumValue]);
  },

  wgpuInstanceGetWGSLLanguageFeatures: (instance, supportedFeatures) => {
    // Always allocate enough space for all the features, though some may be unused.
    var featuresPtr = _malloc(navigator["gpu"]["wgslLanguageFeatures"].size * 4);
    var offset = 0;
    var numFeatures = 0;
    navigator["gpu"]["wgslLanguageFeatures"].forEach(feature => {
      var featureEnumValue = WebGPU.WGSLLanguageFeatureNameString2Enum[feature];
      if (featureEnumValue !== undefined) {
        {{{ makeSetValue('featuresPtr', 'offset', 'featureEnumValue', 'i32') }}};
        offset += 4;
        numFeatures++;
      }
    });
    {{{ makeSetValue('supportedFeatures', C_STRUCTS.WGPUSupportedWGSLLanguageFeatures.features, 'featuresPtr', '*') }}};
    {{{ makeSetValue('supportedFeatures', C_STRUCTS.WGPUSupportedWGSLLanguageFeatures.featureCount, 'numFeatures', '*') }}};
  },

  emwgpuInstanceRequestAdapter__deps: ['emwgpuOnRequestAdapterCompleted'],
  emwgpuInstanceRequestAdapter__sig: 'vpjpp',
  emwgpuInstanceRequestAdapter: (instancePtr, futureId, options, adapterPtr) => {
    var opts;
    if (options) {
      {{{ gpu.makeCheck('options') }}}
      var featureLevel = {{{ gpu.makeGetU32('options', C_STRUCTS.WGPURequestAdapterOptions.featureLevel) }}};
      opts = {
        "featureLevel": WebGPU.FeatureLevel[featureLevel],
        "powerPreference": WebGPU.PowerPreference[
          {{{ gpu.makeGetU32('options', C_STRUCTS.WGPURequestAdapterOptions.powerPreference) }}}],
        "forceFallbackAdapter":
          {{{ gpu.makeGetBool('options', C_STRUCTS.WGPURequestAdapterOptions.forceFallbackAdapter) }}},
      };

      var nextInChainPtr = {{{ makeGetValue('options', C_STRUCTS.WGPURequestAdapterOptions.nextInChain, '*') }}};
      if (nextInChainPtr !== 0) {
        var sType = {{{ gpu.makeGetU32('nextInChainPtr', C_STRUCTS.WGPUChainedStruct.sType) }}};
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

    {{{ runtimeKeepalivePush() }}}
    WebGPU.Internals.futureInsert(futureId, navigator["gpu"]["requestAdapter"](opts).then((adapter) => {
      {{{ runtimeKeepalivePop() }}}
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
    }, (ex) => {
      {{{ runtimeKeepalivePop() }}}
      var sp = stackSave();
      var messagePtr = stringToUTF8OnStack(ex.message);
      _emwgpuOnRequestAdapterCompleted(futureId, {{{ gpu.RequestAdapterStatus.Error }}},
        {{{ gpu.passAsPointer('adapterPtr') }}}, {{{ gpu.passAsPointer('messagePtr') }}});
      stackRestore(sp);
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

  wgpuQuerySetGetType: (querySetPtr, labelPtr) => {
    var querySet = WebGPU.getJsObject(querySetPtr);
    return querySet.type;
  },

  // --------------------------------------------------------------------------
  // Methods of Queue
  // --------------------------------------------------------------------------

  emwgpuQueueOnSubmittedWorkDone__deps: ['emwgpuOnWorkDoneCompleted'],
  emwgpuQueueOnSubmittedWorkDone__sig: 'vpj',
  emwgpuQueueOnSubmittedWorkDone: (queuePtr, futureId) => {
    var queue = WebGPU.getJsObject(queuePtr);

    {{{ runtimeKeepalivePush() }}}
    WebGPU.Internals.futureInsert(futureId, queue.onSubmittedWorkDone().then(() => {
      {{{ runtimeKeepalivePop() }}}
      _emwgpuOnWorkDoneCompleted(futureId, {{{ gpu.QueueWorkDoneStatus.Success }}});
    }, () => {
      {{{ runtimeKeepalivePop() }}}
      abort('Unexpected failure in GPUQueue.onSubmittedWorkDone().')
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
    var pass = WebGPU.getJsObject(passPtr);
    pass.draw(vertexCount, instanceCount, firstVertex, firstInstance);
  },

  wgpuRenderBundleEncoderDrawIndexed: (passPtr, indexCount, instanceCount, firstIndex, baseVertex, firstInstance) => {
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

  wgpuRenderBundleEncoderInsertDebugMarker2: (encoderPtr, markerLabelPtr) => {
    var encoder = WebGPU.getJsObject(encoderPtr);
    encoder.insertDebugMarker(WebGPU.makeStringFromStringView(markerLabelPtr));
  },

  wgpuRenderBundleEncoderPopDebugGroup: (encoderPtr) => {
    var encoder = WebGPU.getJsObject(encoderPtr);
    encoder.popDebugGroup();
  },

  wgpuRenderBundleEncoderPushDebugGroup2: (encoderPtr, groupLabelPtr) => {
    var encoder = WebGPU.getJsObject(encoderPtr);
    encoder.pushDebugGroup(WebGPU.makeStringFromStringView(groupLabelPtr));
  },

  wgpuRenderBundleEncoderSetBindGroup: (passPtr, groupIndex, groupPtr, dynamicOffsetCount, dynamicOffsetsPtr) => {
    var pass = WebGPU.getJsObject(passPtr);
    var group = WebGPU.getJsObject(groupPtr);
    if (dynamicOffsetCount == 0) {
      pass.setBindGroup(groupIndex, group);
    } else {
      var offsets = [];
      for (var i = 0; i < dynamicOffsetCount; i++, dynamicOffsetsPtr += 4) {
        offsets.push({{{ gpu.makeGetU32('dynamicOffsetsPtr', 0) }}});
      }
      pass.setBindGroup(groupIndex, group, offsets);
    }
  },

  wgpuRenderBundleEncoderSetIndexBuffer: (passPtr, bufferPtr, format, offset, size) => {
    var pass = WebGPU.getJsObject(passPtr);
    var buffer = WebGPU.getJsObject(bufferPtr);
    {{{ gpu.convertSentinelToUndefined('size') }}}
    pass.setIndexBuffer(buffer, WebGPU.IndexFormat[format], offset, size);
  },

  wgpuRenderBundleEncoderSetPipeline: (passPtr, pipelinePtr) => {
    var pass = WebGPU.getJsObject(passPtr);
    var pipeline = WebGPU.getJsObject(pipelinePtr);
    pass.setPipeline(pipeline);
  },

  wgpuRenderBundleEncoderSetVertexBuffer: (passPtr, slot, bufferPtr, offset, size) => {
    var pass = WebGPU.getJsObject(passPtr);
    var buffer = WebGPU.getJsObject(bufferPtr);
    {{{ gpu.convertSentinelToUndefined('size') }}}
    pass.setVertexBuffer(slot, buffer, offset, size);
  },

  // --------------------------------------------------------------------------
  // Methods of RenderPassEncoder
  // --------------------------------------------------------------------------

  wgpuRenderPassEncoderBeginOcclusionQuery: (passPtr, queryIndex) => {
    var pass = WebGPU.getJsObject(passPtr);
    pass.beginOcclusionQuery(queryIndex);
  },

  wgpuRenderPassEncoderDraw: (passPtr, vertexCount, instanceCount, firstVertex, firstInstance) => {
    var pass = WebGPU.getJsObject(passPtr);
    pass.draw(vertexCount, instanceCount, firstVertex, firstInstance);
  },

  wgpuRenderPassEncoderDrawIndexed: (passPtr, indexCount, instanceCount, firstIndex, baseVertex, firstInstance) => {
    var pass = WebGPU.getJsObject(passPtr);
    pass.drawIndexed(indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
  },

  wgpuRenderPassEncoderDrawIndexedIndirect: (passPtr, indirectBufferPtr, indirectOffset) => {
    var indirectBuffer = WebGPU.getJsObject(indirectBufferPtr);
    var pass = WebGPU.getJsObject(passPtr);
    pass.drawIndexedIndirect(indirectBuffer, indirectOffset);
  },

  wgpuRenderPassEncoderDrawIndirect: (passPtr, indirectBufferPtr, indirectOffset) => {
    var indirectBuffer = WebGPU.getJsObject(indirectBufferPtr);
    var pass = WebGPU.getJsObject(passPtr);
    pass.drawIndirect(indirectBuffer, indirectOffset);
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

  wgpuRenderPassEncoderInsertDebugMarker2: (encoderPtr, markerLabelPtr) => {
    var encoder = WebGPU.getJsObject(encoderPtr);
    encoder.insertDebugMarker(WebGPU.makeStringFromStringView(markerLabelPtr));
  },

  wgpuRenderPassEncoderPopDebugGroup: (encoderPtr) => {
    var encoder = WebGPU.getJsObject(encoderPtr);
    encoder.popDebugGroup();
  },

  wgpuRenderPassEncoderPushDebugGroup2: (encoderPtr, groupLabelPtr) => {
    var encoder = WebGPU.getJsObject(encoderPtr);
    encoder.pushDebugGroup(WebGPU.makeStringFromStringView(groupLabelPtr));
  },

  wgpuRenderPassEncoderSetBindGroup: (passPtr, groupIndex, groupPtr, dynamicOffsetCount, dynamicOffsetsPtr) => {
    var pass = WebGPU.getJsObject(passPtr);
    var group = WebGPU.getJsObject(groupPtr);
    if (dynamicOffsetCount == 0) {
      pass.setBindGroup(groupIndex, group);
    } else {
      var offsets = [];
      for (var i = 0; i < dynamicOffsetCount; i++, dynamicOffsetsPtr += 4) {
        offsets.push({{{ gpu.makeGetU32('dynamicOffsetsPtr', 0) }}});
      }
      pass.setBindGroup(groupIndex, group, offsets);
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
    {{{ gpu.convertSentinelToUndefined('size') }}}
    pass.setIndexBuffer(buffer, WebGPU.IndexFormat[format], offset, size);
  },

  wgpuRenderPassEncoderSetPipeline: (passPtr, pipelinePtr) => {
    var pass = WebGPU.getJsObject(passPtr);
    var pipeline = WebGPU.getJsObject(pipelinePtr);
    pass.setPipeline(pipeline);
  },

  wgpuRenderPassEncoderSetScissorRect: (passPtr, x, y, w, h) => {
    var pass = WebGPU.getJsObject(passPtr);
    pass.setScissorRect(x, y, w, h);
  },

  wgpuRenderPassEncoderSetStencilReference: (passPtr, reference) => {
    var pass = WebGPU.getJsObject(passPtr);
    pass.setStencilReference(reference);
  },

  wgpuRenderPassEncoderSetVertexBuffer: (passPtr, slot, bufferPtr, offset, size) => {
    var pass = WebGPU.getJsObject(passPtr);
    var buffer = WebGPU.getJsObject(bufferPtr);
    {{{ gpu.convertSentinelToUndefined('size') }}}
    pass.setVertexBuffer(slot, buffer, offset, size);
  },

  wgpuRenderPassEncoderSetViewport: (passPtr, x, y, w, h, minDepth, maxDepth) => {
    var pass = WebGPU.getJsObject(passPtr);
    pass.setViewport(x, y, w, h, minDepth, maxDepth);
  },

  wgpuRenderPassEncoderWriteTimestamp: (encoderPtr, querySetPtr, queryIndex) => {
    var encoder = WebGPU.getJsObject(encoderPtr);
    var querySet = WebGPU.getJsObject(querySetPtr);
    encoder.writeTimestamp(querySet, queryIndex);
  },

  // --------------------------------------------------------------------------
  // Methods of RenderPipeline
  // --------------------------------------------------------------------------

  wgpuRenderPipelineGetBindGroupLayout__deps: ['emwgpuCreateBindGroupLayout'],
  wgpuRenderPipelineGetBindGroupLayout: (pipelinePtr, groupIndex) => {
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

  emwgpuShaderModuleGetCompilationInfo__deps: ['emwgpuOnCompilationInfoCompleted', '$stringToUTF8', '$lengthBytesUTF8', 'malloc'],
  emwgpuShaderModuleGetCompilationInfo__sig: 'vpjp',
  emwgpuShaderModuleGetCompilationInfo: (shaderModulePtr, futureId, compilationInfoPtr) => {
    var shaderModule = WebGPU.getJsObject(shaderModulePtr);
    {{{ runtimeKeepalivePush() }}}
    WebGPU.Internals.futureInsert(futureId, shaderModule.getCompilationInfo().then((compilationInfo) => {
      {{{ runtimeKeepalivePop() }}}
      // Calculate the total length of strings and offsets here to malloc them
      // all at once. Note that we start at 1 instead of 0 for the total size
      // to ensure there's enough space for the null terminator that is always
      // added by stringToUTF8.
      var totalMessagesSize = 1;
      var messageLengths = [];
      for (var i = 0; i < compilationInfo.messages.length; ++i) {
        var messageLength = lengthBytesUTF8(compilationInfo.messages[i].message);
        totalMessagesSize += messageLength;
        messageLengths.push(messageLength);
      }
      var messagesPtr = _malloc(totalMessagesSize);

      // Allocate and fill out each CompilationMessage.
      var compilationMessagesPtr = _malloc({{{ C_STRUCTS.WGPUCompilationMessage.__size__ }}} * compilationInfo.messages.length);
      var utf16sPtr = _malloc({{{ C_STRUCTS.WGPUDawnCompilationMessageUtf16.__size__ }}} * compilationInfo.messages.length);
      for (var i = 0; i < compilationInfo.messages.length; ++i) {
        var compilationMessage = compilationInfo.messages[i];
        var compilationMessagePtr = compilationMessagesPtr + {{{ C_STRUCTS.WGPUCompilationMessage.__size__ }}} * i;
        var utf16Ptr = utf16sPtr + {{{ C_STRUCTS.WGPUDawnCompilationMessageUtf16.__size__ }}} * i;

        // Write out the values to the CompilationMessage.
        WebGPU.setStringView(compilationMessagePtr + {{{ C_STRUCTS.WGPUCompilationMessage.message }}}, messagesPtr, messageLengths[i]);
        // TODO: Convert JavaScript's UTF-16-code-unit offsets to UTF-8-code-unit offsets.
        // https://github.com/webgpu-native/webgpu-headers/issues/246
        {{{ makeSetValue('compilationMessagePtr', C_STRUCTS.WGPUCompilationMessage.nextInChain, 'utf16Ptr', '*') }}};
        {{{ makeSetValue('compilationMessagePtr', C_STRUCTS.WGPUCompilationMessage.type, 'WebGPU.Int_CompilationMessageType[compilationMessage.type]', 'i32') }}};
        {{{ makeSetValue('compilationMessagePtr', C_STRUCTS.WGPUCompilationMessage.lineNum, 'compilationMessage.lineNum', 'i64') }}};
        {{{ makeSetValue('compilationMessagePtr', C_STRUCTS.WGPUCompilationMessage.linePos, 'compilationMessage.linePos', 'i64') }}};
        {{{ makeSetValue('compilationMessagePtr', C_STRUCTS.WGPUCompilationMessage.offset, 'compilationMessage.offset', 'i64') }}};
        {{{ makeSetValue('compilationMessagePtr', C_STRUCTS.WGPUCompilationMessage.length, 'compilationMessage.length', 'i64') }}};

        {{{ makeSetValue('utf16Ptr', C_STRUCTS.WGPUChainedStruct.next, '0', '*') }}};
        {{{ makeSetValue('utf16Ptr', C_STRUCTS.WGPUChainedStruct.sType, gpu.SType.DawnCompilationMessageUtf16, 'i32') }}};
        {{{ makeSetValue('utf16Ptr', C_STRUCTS.WGPUDawnCompilationMessageUtf16.linePos, 'compilationMessage.linePos', 'i64') }}};
        {{{ makeSetValue('utf16Ptr', C_STRUCTS.WGPUDawnCompilationMessageUtf16.offset, 'compilationMessage.offset', 'i64') }}};
        {{{ makeSetValue('utf16Ptr', C_STRUCTS.WGPUDawnCompilationMessageUtf16.length, 'compilationMessage.length', 'i64') }}};

        // Write the string out to the allocated buffer. Note we have to add 1
        // to the length of the string to ensure enough space for the null
        // terminator. However, we only increment the pointer by the exact
        // length so we overwrite the null terminators except for the last one.
        stringToUTF8(compilationMessage.message, messagesPtr, messageLengths[i] + 1);
        messagesPtr += messageLengths[i];
      }

      // Allocate and fill out the wrapping CompilationInfo struct.
      {{{ makeSetValue('compilationInfoPtr', C_STRUCTS.WGPUCompilationInfo.messageCount, 'compilationInfo.messages.length', '*') }}}
      {{{ makeSetValue('compilationInfoPtr', C_STRUCTS.WGPUCompilationInfo.messages, 'compilationMessagesPtr', '*') }}};

      _emwgpuOnCompilationInfoCompleted(futureId, {{{ gpu.CompilationInfoRequestStatus.Success }}}, {{{ gpu.passAsPointer('compilationInfoPtr') }}});
    }, () => {
      abort('Unexpected failure in GPUShaderModule.getCompilationInfo().')
    }));
  },

  // --------------------------------------------------------------------------
  // Methods of Surface
  // --------------------------------------------------------------------------

  wgpuSurfaceConfigure: (surfacePtr, config) => {
    {{{ gpu.makeCheck('config') }}}
    var devicePtr = {{{ makeGetValue('config', C_STRUCTS.WGPUSurfaceConfiguration.device, '*') }}};
    var context = WebGPU.getJsObject(surfacePtr);

#if ASSERTIONS
    assert({{{ gpu.PresentMode.Fifo }}} ===
      {{{ gpu.makeGetU32('config', C_STRUCTS.WGPUSurfaceConfiguration.presentMode) }}});
#endif

    var canvasSize = [
      {{{ gpu.makeGetU32('config', C_STRUCTS.WGPUSurfaceConfiguration.width) }}},
      {{{ gpu.makeGetU32('config', C_STRUCTS.WGPUSurfaceConfiguration.height) }}}
    ];

    if (canvasSize[0] !== 0) {
      context["canvas"]["width"] = canvasSize[0];
    }

    if (canvasSize[1] !== 0) {
      context["canvas"]["height"] = canvasSize[1];
    }

    var configuration = {
      "device": WebGPU.getJsObject(devicePtr),
      "format": WebGPU.TextureFormat[
        {{{ gpu.makeGetU32('config', C_STRUCTS.WGPUSurfaceConfiguration.format) }}}],
      "usage": {{{ gpu.makeGetU32('config', C_STRUCTS.WGPUSurfaceConfiguration.usage) }}},
      "alphaMode": WebGPU.CompositeAlphaMode[
        {{{ gpu.makeGetU32('config', C_STRUCTS.WGPUSurfaceConfiguration.alphaMode) }}}],
    };

    var viewFormatCount = {{{ gpu.makeGetU32('config', C_STRUCTS.WGPUSurfaceConfiguration.viewFormatCount) }}};
    if (viewFormatCount) {
      var viewFormatsPtr = {{{ makeGetValue('config', C_STRUCTS.WGPUSurfaceConfiguration.viewFormats, '*') }}};
      // viewFormatsPtr pointer to an array of TextureFormat which is an enum of size uint32_t
      configuration['viewFormats'] = Array.from({{{ makeHEAPView('32', 'viewFormatsPtr', 'viewFormatsPtr + viewFormatCount * 4') }}},
        format => WebGPU.TextureFormat[format]);
    }

    {
      var nextInChainPtr = {{{ makeGetValue('config', C_STRUCTS.WGPUSurfaceConfiguration.nextInChain, '*') }}};

      if (nextInChainPtr !== 0) {
        var sType = {{{ gpu.makeGetU32('nextInChainPtr', C_STRUCTS.WGPUChainedStruct.sType) }}};
#if ASSERTIONS
        assert(sType === {{{ gpu.SType.SurfaceColorManagement }}});
        assert(0 === {{{ makeGetValue('nextInChainPtr', gpu.kOffsetOfNextInChainMember, '*') }}});
#endif
        var surfaceColorManagement = nextInChainPtr;
        {{{ gpu.makeCheckDescriptor('surfaceColorManagement') }}}
        configuration.colorSpace = WebGPU.PredefinedColorSpace[
          {{{ gpu.makeGetU32('surfaceColorManagement', C_STRUCTS.WGPUSurfaceColorManagement.colorSpace) }}}];
        configuration.toneMapping = {
          mode: WebGPU.ToneMappingMode[
            {{{ gpu.makeGetU32('surfaceColorManagement', C_STRUCTS.WGPUSurfaceColorManagement.toneMappingMode) }}}],
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
      {{{ gpu.makeCheckDescriptor('descriptor') }}}
      var mipLevelCount = {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.mipLevelCount) }}};
      var arrayLayerCount = {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.arrayLayerCount) }}};
      desc = {
        "label": WebGPU.makeStringFromOptionalStringView(
          descriptor + {{{ C_STRUCTS.WGPUTextureViewDescriptor.label }}}),
        "format": WebGPU.TextureFormat[
          {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.format) }}}],
        "dimension": WebGPU.TextureViewDimension[
          {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.dimension) }}}],
        "baseMipLevel": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.baseMipLevel) }}},
        "mipLevelCount": mipLevelCount === {{{ gpu.MIP_LEVEL_COUNT_UNDEFINED }}} ? undefined : mipLevelCount,
        "baseArrayLayer": {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.baseArrayLayer) }}},
        "arrayLayerCount": arrayLayerCount === {{{ gpu.ARRAY_LAYER_COUNT_UNDEFINED }}} ? undefined : arrayLayerCount,
        "aspect": WebGPU.TextureAspect[
          {{{ gpu.makeGetU32('descriptor', C_STRUCTS.WGPUTextureViewDescriptor.aspect) }}}],
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

// Inverted index used by GetFeatures/HasFeature
LibraryWebGPU.$WebGPU.FeatureNameString2Enum = {};
for (var value in LibraryWebGPU.$WebGPU.FeatureName) {
  LibraryWebGPU.$WebGPU.FeatureNameString2Enum[LibraryWebGPU.$WebGPU.FeatureName[value]] = value;
}
LibraryWebGPU.$WebGPU.WGSLLanguageFeatureNameString2Enum = {};
for (var value in LibraryWebGPU.$WebGPU.WGSLLanguageFeatureName) {
  LibraryWebGPU.$WebGPU.WGSLLanguageFeatureNameString2Enum[LibraryWebGPU.$WebGPU.WGSLLanguageFeatureName[value]] = value;
}

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
moveDeps(LibraryWebGPU.$WebGPU, LibraryWebGPU.$WebGPU__deps)

autoAddDeps(LibraryWebGPU, '$WebGPU');
mergeInto(LibraryManager.library, LibraryWebGPU);
