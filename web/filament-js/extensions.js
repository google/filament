/*
* Copyright (C) 2018 The Android Open Source Project
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

// Private utility that converts an asset string or Uint8Array into a low-level buffer descriptor.
// Note that the low-level buffer descriptor must be manually deleted.
function getBufferDescriptor(buffer) {
    if ('string' == typeof buffer || buffer instanceof String) {
        buffer = Filament.assets[buffer];
    }
    if (buffer.buffer instanceof ArrayBuffer) {
        buffer = Filament.Buffer(buffer);
    }
    return buffer;
}

function isTexture(uri) {
    // TODO: This is not a great way to determine if a resource is a texture, but we can
    // remove it after gltfio gains support for concurrent downloading of vertex data:
    // https://github.com/google/filament/issues/5909
    if (uri.endsWith(".png")) {
        return true;
    }
    if (uri.endsWith(".ktx2")) {
        return true;
    }
    if (uri.endsWith(".jpg") || uri.endsWith(".jpeg")) {
        return true;
    }
    return false;
}

Filament.vectorToArray = function(vector) {
    const result = [];
    for (let i = 0; i < vector.size(); i++) {
        result.push(vector.get(i));
    }
    return result;
};

Filament.shadowOptions = function(overrides) {
    const options = {
        mapSize: 1024,
        shadowCascades: 1,
        constantBias: 0.001,
        normalBias: 1.0,
        shadowFar: 0.0,
        shadowNearHint: 1.0,
        shadowFarHint: 100.0,
        stable: false,
        polygonOffsetConstant: 0.5,
        polygonOffsetSlope: 2.0,
        screenSpaceContactShadows: false,
        stepCount: 8,
        maxShadowDistance: 0.3
    };
    return Object.assign(options, overrides);
};

Filament.loadClassExtensions = function() {

    Filament.loadGeneratedExtensions();

    /// Engine ::core class::

    /// create ::static method:: Creates an Engine instance for the given canvas.
    /// canvas ::argument:: the canvas DOM element
    /// options ::argument:: optional WebGL 2.0 context configuration
    /// ::retval:: an instance of [Engine]
    Filament.Engine.create = function(canvas, options) {
        const defaults = {
            majorVersion: 2,
            minorVersion: 0,
            antialias: false,
            depth: true,
            alpha: false
        };
        options = Object.assign(defaults, options);

        // Create the WebGL 2.0 context.
        const ctx = canvas.getContext("webgl2", options);

        // Enable all desired extensions by calling getExtension on each one.
        ctx.getExtension('WEBGL_compressed_texture_s3tc');
        ctx.getExtension('WEBGL_compressed_texture_s3tc_srgb');
        ctx.getExtension('WEBGL_compressed_texture_astc');
        ctx.getExtension('WEBGL_compressed_texture_etc');

        // These transient globals are used temporarily during Engine construction.
        window.filament_glOptions = options;
        window.filament_glContext = ctx;

        // Register the GL context with emscripten and create the Engine.
        const engine = Filament.Engine._create();

        // Annotate the engine with the GL context to support multiple canvases.
        engine.context = window.filament_glContext;
        engine.handle = window.filament_contextHandle;

        // Ensure that we do not pollute the global namespace.
        delete window.filament_glOptions;
        delete window.filament_glContext;
        delete window.filament_contextHandle;

        return engine;
    };

    Filament.Engine.prototype.execute = function() {
        window.filament_contextHandle = this.handle;
        this._execute();
        delete window.filament_contextHandle;
    };

    /// createMaterial ::method::
    /// package ::argument:: asset string, or Uint8Array, or [Buffer] with filamat contents
    /// ::retval:: an instance of [createMaterial]
    Filament.Engine.prototype.createMaterial = function(buffer) {
        buffer = getBufferDescriptor(buffer);
        const result = this._createMaterial(buffer);
        buffer.delete();
        return result;
    };

    /// createTextureFromKtx1 ::method:: Utility function that creates a [Texture] from a KTX1 file.
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer] with KTX1 file contents
    /// options ::argument:: Options dictionary.
    /// ::retval:: [Texture]
    Filament.Engine.prototype.createTextureFromKtx1 = function(buffer, options) {
        buffer = getBufferDescriptor(buffer);
        const result = Filament._createTextureFromKtx1(buffer, this, options);
        buffer.delete();
        return result;
    };

    /// createTextureFromKtx2 ::method:: Utility function that creates a [Texture] from a KTX2 file.
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer] with KTX2 file contents
    /// options ::argument:: Options dictionary.
    /// ::retval:: [Texture]
    Filament.Engine.prototype.createTextureFromKtx2 = function(buffer, options) {
        options = options || {};
        buffer = getBufferDescriptor(buffer);

        const engine = this;
        const quiet = false;
        const reader = new Filament.Ktx2Reader(engine, quiet);

        reader.requestFormat(Filament.Texture$InternalFormat.RGBA8);
        reader.requestFormat(Filament.Texture$InternalFormat.SRGB8_A8);

        const formats = options.formats || [];
        for (const format of formats) {
            reader.requestFormat(format);
        }

        result = reader.load(buffer, options.srgb ? Filament.Ktx2Reader$TransferFunction.sRGB :
            Filament.Ktx2Reader$TransferFunction.LINEAR);

        reader.delete();
        buffer.delete();
        return result;
    };

    /// createIblFromKtx1 ::method:: Utility that creates an [IndirectLight] from a KTX file.
    /// NOTE: To prevent a leak, please be sure to destroy the associated reflections texture.
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer] with KTX file contents
    /// options ::argument:: Options dictionary.
    /// ::retval:: [IndirectLight]
    Filament.Engine.prototype.createIblFromKtx1 = function(buffer, options) {
        buffer = getBufferDescriptor(buffer);
        const result = Filament._createIblFromKtx1(buffer, this, options);
        buffer.delete();
        return result;
    };

    /// createSkyFromKtx1 ::method:: Utility function that creates a [Skybox] from a KTX file.
    /// NOTE: To prevent a leak, please be sure to destroy the associated texture.
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer] with KTX file contents
    /// options ::argument:: Options dictionary.
    /// ::retval:: [Skybox]
    Filament.Engine.prototype.createSkyFromKtx1 = function(buffer, options) {
        const skytex = this.createTextureFromKtx1(buffer, options);
        return Filament.Skybox.Builder().environment(skytex).build(this);
    };

    /// createTextureFromPng ::method:: Creates a 2D [Texture] from the raw contents of a PNG file.
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer] with PNG file contents
    /// options ::argument:: object with optional `srgb`, `noalpha`, and `nomips` keys.
    /// ::retval:: [Texture]
    Filament.Engine.prototype.createTextureFromPng = function(buffer, options) {
        buffer = getBufferDescriptor(buffer);
        const result = Filament._createTextureFromImageFile(buffer, this, options);
        buffer.delete();
        return result;
    };

    /// createTextureFromJpeg ::method:: Creates a 2D [Texture] from the contents of a JPEG file.
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer] with JPEG file contents
    /// options ::argument:: JavaScript object with optional `srgb` and `nomips` keys.
    /// ::retval:: [Texture]
    Filament.Engine.prototype.createTextureFromJpeg = function(buffer, options) {
        buffer = getBufferDescriptor(buffer);
        const result = Filament._createTextureFromImageFile(buffer, this, options);
        buffer.delete();
        return result;
    };

    /// loadFilamesh ::method:: Consumes the contents of a filamesh file and creates a renderable.
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer] with filamesh contents
    /// definstance ::argument:: Optional default [MaterialInstance]
    /// matinstances ::argument:: Optional in-out object that gets populated with a \
    /// name-to-[MaterialInstance] mapping. Clients can also optionally provide individual \
    /// material instances using this argument.
    /// ::retval:: JavaScript object with keys `renderable`, `vertexBuffer`, and `indexBuffer`. \
    /// These are of type [Entity], [VertexBuffer], and [IndexBuffer].
    Filament.Engine.prototype.loadFilamesh = function(buffer, definstance, matinstances) {
        buffer = getBufferDescriptor(buffer);
        const result = Filament._loadFilamesh(this, buffer, definstance, matinstances);
        buffer.delete();
        return result;
    };

    /// createAssetLoader ::method::
    /// ::retval:: an instance of [AssetLoader]
    /// Clients should create only one asset loader for the lifetime of their app, this prevents
    /// memory leaks and duplication of Material objects.
    Filament.Engine.prototype.createAssetLoader = function() {
        const materials = new Filament.gltfio$UbershaderProvider(this);
        return new Filament.gltfio$AssetLoader(this, materials);
    };

    /// addEntities ::method::
    /// entities ::argument:: array of entities
    /// This method is equivalent to calling `addEntity` on each item in the array.
    Filament.Scene.prototype.addEntities = function(entities) {
        const vector = new Filament.EntityVector();
        for (const entity of entities) {
            vector.push_back(entity);
        }
        this._addEntities(vector);
    };

    /// removeEntities ::method::
    /// entities ::argument:: array of entities
    /// This method is equivalent to calling `remove` on each item in the array.
    Filament.Scene.prototype.removeEntities = function(entities) {
        const vector = new Filament.EntityVector();
        for (const entity of entities) {
            vector.push_back(entity);
        }
        this._removeEntities(vector);
    };

    /// setShadowOptions ::method::
    /// instance ::argument:: Instance of a light component obtained from `getInstance`.
    /// overrides ::argument:: Dictionary with one or more of the following properties: \
    /// mapSize, shadowCascades, constantBias, normalBias, shadowFar, shadowNearHint, \
    /// shadowFarHint, stable, polygonOffsetConstant, polygonOffsetSlope, \
    // screenSpaceContactShadows, stepCount, maxShadowDistance.
    Filament.LightManager.prototype.setShadowOptions = function(instance, overrides) {
        this._setShadowOptions(instance, Filament.shadowOptions(overrides));
    };

    /// setClearOptions ::method::
    /// overrides ::argument:: Dictionary with one or more of the following properties: \
    /// clearColor, clear, discard.
    Filament.Renderer.prototype.setClearOptions = function(overrides) {
        const options = {
            clearColor: [0, 0, 0, 0],
            clear: false,
            discard: true
        };
        Object.assign(options, overrides);
        this._setClearOptions(options);
    };

    /// setAmbientOcclusionOptions ::method::
    Filament.View.prototype.setAmbientOcclusionOptions = function(overrides) {
        const options = this.setAmbientOcclusionOptionsDefaults(overrides);
        this._setAmbientOcclusionOptions(options);
    };

    /// setDepthOfFieldOptions ::method::
    Filament.View.prototype.setDepthOfFieldOptions = function(overrides) {
        const options = this.setDepthOfFieldOptionsDefaults(overrides);
        this._setDepthOfFieldOptions(options);
    };

    /// setMultiSampleAntiAliasingOptions ::method::
    Filament.View.prototype.setMultiSampleAntiAliasingOptions = function(overrides) {
        const options = this.setMultiSampleAntiAliasingOptionsDefaults(overrides);
        this._setMultiSampleAntiAliasingOptions(options);
    };

    /// setTemporalAntiAliasingOptions ::method::
    Filament.View.prototype.setTemporalAntiAliasingOptions = function(overrides) {
        const options = this.setTemporalAntiAliasingOptionsDefaults(overrides);
        this._setTemporalAntiAliasingOptions(options);
    };

    /// setScreenSpaceReflectionsOptions ::method::
    Filament.View.prototype.setScreenSpaceReflectionsOptions = function(overrides) {
        const options = this.setScreenSpaceReflectionsOptionsDefaults(overrides);
        this._setScreenSpaceReflectionsOptions(options);
    };

    /// setBloomOptions ::method::
    Filament.View.prototype.setBloomOptions = function(overrides) {
        const options = this.setBloomOptionsDefaults(overrides);
        this._setBloomOptions(options);
    };

    /// setFogOptions ::method::
    Filament.View.prototype.setFogOptions = function(overrides) {
        const options = this.setFogOptionsDefaults(overrides);
        this._setFogOptions(options);
    };

    /// setVignetteOptions ::method::
    Filament.View.prototype.setVignetteOptions = function(overrides) {
        const options = this.setVignetteOptionsDefaults(overrides);
        this._setVignetteOptions(options);
    };

    /// setGuardBandOptions ::method::
    Filament.View.prototype.setGuardBandOptions = function(overrides) {
        const options = this.setGuardBandOptionsDefaults(overrides);
        this._setGuardBandOptions(options);
    };

    /// setStereoscopicOptions ::method::
    Filament.View.prototype.setStereoscopicOptions = function(overrides) {
        const options = this.setStereoscopicOptionsDefaults(overrides);
        this._setStereoscopicOptions(options);
    }

    /// BufferObject ::core class::

    /// setBuffer ::method::
    /// engine ::argument:: [Engine]
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer]
    /// byteOffset ::argument:: non-negative integer
    Filament.BufferObject.prototype.setBuffer = function(engine, buffer, byteOffset = 0) {
        buffer = getBufferDescriptor(buffer);
        this._setBuffer(engine, buffer, byteOffset);
        buffer.delete();
    };

    /// VertexBuffer ::core class::

    /// setBufferAt ::method::
    /// engine ::argument:: [Engine]
    /// bufferIndex ::argument:: non-negative integer
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer]
    /// byteOffset ::argument:: non-negative integer
    Filament.VertexBuffer.prototype.setBufferAt = function(engine, bufferIndex, buffer, byteOffset = 0) {
        buffer = getBufferDescriptor(buffer);
        this._setBufferAt(engine, bufferIndex, buffer, byteOffset);
        buffer.delete();
    };

    /// IndexBuffer ::core class::

    /// setBuffer ::method::
    /// engine ::argument:: [Engine]
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer]
    /// byteOffset ::argument:: non-negative integer
    Filament.IndexBuffer.prototype.setBuffer = function(engine, buffer, byteOffset = 0) {
        buffer = getBufferDescriptor(buffer);
        this._setBuffer(engine, buffer, byteOffset);
        buffer.delete();
    };

    Filament.LightManager$Builder.prototype.shadowOptions = function(overrides) {
        return this._shadowOptions(Filament.shadowOptions(overrides));
    };

    Filament.RenderableManager$Builder.prototype.build =
    Filament.LightManager$Builder.prototype.build =
        function(engine, entity) {
            const result = this._build(engine, entity);
            this.delete();
            return result;
        };

    Filament.ColorGrading$Builder.prototype.build =
    Filament.RenderTarget$Builder.prototype.build =
    Filament.VertexBuffer$Builder.prototype.build =
    Filament.IndexBuffer$Builder.prototype.build =
    Filament.Texture$Builder.prototype.build =
    Filament.IndirectLight$Builder.prototype.build =
    Filament.Skybox$Builder.prototype.build =
        function(engine) {
            const result = this._build(engine);
            this.delete();
            return result;
        };

    Filament.Ktx1Bundle.prototype.getBlob = function(index) {
        const blob = this._getBlob(index);
        const result = blob.getBytes();
        blob.delete();
        return result;
    }

    Filament.Ktx1Bundle.prototype.getCubeBlob = function(miplevel) {
        const blob = this._getCubeBlob(miplevel);
        const result = blob.getBytes();
        blob.delete();
        return result;
    }

    Filament.Texture.prototype.setImage = function(engine, level, pbd) {
        this._setImage(engine, level, pbd);
        pbd.delete();
    }

    Filament.Texture.prototype.setImageCube = function(engine, level, pbd) {
        this._setImageCube(engine, level, pbd);
        pbd.delete();
    }

    Filament.Texture.prototype.getWidth = function(engine, level = 0) {
        return this._getWidth(engine, level);
    }

    Filament.Texture.prototype.getHeight = function(engine, level = 0) {
        return this._getHeight(engine, level);
    }

    Filament.Texture.prototype.getDepth = function(engine, level = 0) {
        return this._getDepth(engine, level);
    }

    Filament.Texture.prototype.getLevels = function(engine) {
        return this._getLevels(engine);
    }

    Filament.SurfaceOrientation$Builder.prototype.normals = function(buffer, stride = 0) {
        buffer = new Uint8Array(buffer.buffer, buffer.byteOffset, buffer.byteLength);
        this.norPointer = Filament._malloc(buffer.byteLength);
        Filament.HEAPU8.set(buffer, this.norPointer);
        this._normals(this.norPointer, stride);
    };

    Filament.SurfaceOrientation$Builder.prototype.uvs = function(buffer, stride = 0) {
        buffer = new Uint8Array(buffer.buffer, buffer.byteOffset, buffer.byteLength);
        this.uvsPointer = Filament._malloc(buffer.byteLength);
        Filament.HEAPU8.set(buffer, this.uvsPointer);
        this._uvs(this.uvsPointer, stride);
    };

    Filament.SurfaceOrientation$Builder.prototype.positions = function(buffer, stride = 0) {
        buffer = new Uint8Array(buffer.buffer, buffer.byteOffset, buffer.byteLength);
        this.posPointer = Filament._malloc(buffer.byteLength);
        Filament.HEAPU8.set(buffer, this.posPointer);
        this._positions(this.posPointer, stride);
    };

    Filament.SurfaceOrientation$Builder.prototype.triangles16 = function(buffer) {
        buffer = new Uint8Array(buffer.buffer, buffer.byteOffset, buffer.byteLength);
        this.t16Pointer = Filament._malloc(buffer.byteLength);
        Filament.HEAPU8.set(buffer, this.t16Pointer);
        this._triangles16(this.t16Pointer);
    };

    Filament.SurfaceOrientation$Builder.prototype.triangles32 = function(buffer) {
        buffer = new Uint8Array(buffer.buffer, buffer.byteOffset, buffer.byteLength);
        this.t32Pointer = Filament._malloc(buffer.byteLength);
        Filament.HEAPU8.set(buffer, this.t32Pointer);
        this._triangles32(this.t32Pointer);
    };

    Filament.SurfaceOrientation$Builder.prototype.build = function() {
        const result = this._build();
        this.delete();
        if ('norPointer' in this) Filament._free(this.norPointer);
        if ('uvsPointer' in this) Filament._free(this.uvsPointer);
        if ('posPointer' in this) Filament._free(this.posPointer);
        if ('t16Pointer' in this) Filament._free(this.t16Pointer);
        if ('t32Pointer' in this) Filament._free(this.t32Pointer);
        return result;
    };

    Filament.SurfaceOrientation.prototype.getQuats = function(nverts) {
        const attribType = Filament.VertexBuffer$AttributeType.SHORT4;
        const quatsBufferSize = 8 * nverts;
        const quatsBuffer = Filament._malloc(quatsBufferSize);
        this._getQuats(quatsBuffer, nverts, attribType);
        const arrayBuffer = Filament.HEAPU8.subarray(quatsBuffer, quatsBuffer + quatsBufferSize).slice().buffer;
        Filament._free(quatsBuffer);
        return new Int16Array(arrayBuffer);
    };

    Filament.SurfaceOrientation.prototype.getQuatsHalf4 = function (nverts) {
        const attribType = Filament.VertexBuffer$AttributeType.HALF4;
        const quatsBufferSize = 8 * nverts;
        const quatsBuffer = Filament._malloc(quatsBufferSize);
        this._getQuats(quatsBuffer, nverts, attribType);
        const arrayBuffer = Filament.HEAPU8.subarray(quatsBuffer, quatsBuffer + quatsBufferSize).slice().buffer;
        Filament._free(quatsBuffer);
        return new Uint16Array(arrayBuffer);
    };

    Filament.SurfaceOrientation.prototype.getQuatsFloat4 = function (nverts) {
        const attribType = Filament.VertexBuffer$AttributeType.FLOAT4;
        const quatsBufferSize = 16 * nverts;
        const quatsBuffer = Filament._malloc(quatsBufferSize);
        this._getQuats(quatsBuffer, nverts, attribType);
        const arrayBuffer = Filament.HEAPU8.subarray(quatsBuffer, quatsBuffer + quatsBufferSize).slice().buffer;
        Filament._free(quatsBuffer);
        return new Float32Array(arrayBuffer);
    };

    Filament.gltfio$AssetLoader.prototype.createAsset = function(buffer) {
        buffer = getBufferDescriptor(buffer);
        const result = this._createAsset(buffer);
        buffer.delete();
        return result;
    };

    Filament.gltfio$AssetLoader.prototype.createInstancedAsset = function(buffer, instances) {
        buffer = getBufferDescriptor(buffer);
        const asset = this._createInstancedAsset(buffer, instances.length);
        buffer.delete();
        const instancesVector = asset._getAssetInstances();
        for (let i = 0; i < instancesVector.size(); i++) {
            instances[i] = instancesVector.get(i);
        }
        return asset;
    };

    // See the C++ documentation for ResourceLoader and AssetLoader. The JavaScript API differs in
    // that it takes two optional callbacks:
    //
    // - onDone is called after all resources have been downloaded and decoded.
    // - onFetched is called after each resource has finished downloading.
    //
    // Takes an optional base path for resolving the URI strings in the glTF file, which is
    // typically the path to the parent glTF file. The given base path cannot itself be a relative
    // URL, but clients can do the following to resolve a relative URL:
    //    const basePath = '' + new URL(myRelativeUrl, document.location);
    // If the given base path is null, document.location is used as the base.
    //
    // The optional asyncInterval argument allows clients to control how decoding is amortized
    // over time. It represents the number of milliseconds between each texture decoding task.
    //
    // The optional config argument is an object with boolean field `normalizeSkinningWeights`.
    Filament.gltfio$FilamentAsset.prototype.loadResources = function(onDone, onFetched, basePath,
            asyncInterval, config) {
        const asset = this;
        const engine = this.getEngine();
        const interval = asyncInterval || 30;
        const defaults = {
            normalizeSkinningWeights: true,
        };
        config = Object.assign(defaults, config || {});

        basePath = basePath || document.location;
        onFetched = onFetched || ((name) => {});
        onDone = onDone || (() => {});

        // Construct two lists of URI strings to fetch: textures and non-textures.
        let textureUris = new Set();
        let bufferUris = new Set();
        const absoluteToRelativeUri = {};
        for (const relativeUri of this.getResourceUris()) {
            const absoluteUri = '' + new URL(relativeUri, basePath);
            absoluteToRelativeUri[absoluteUri] = relativeUri;
            if (isTexture(relativeUri)) {
                textureUris.add(absoluteUri);
                continue;
            }
            bufferUris.add(absoluteUri);
        }
        textureUris = Array.from(textureUris);
        bufferUris = Array.from(bufferUris);

        // Construct a resource loader and start decoding after all textures are fetched.
        const resourceLoader = new Filament.gltfio$ResourceLoader(engine,
                config.normalizeSkinningWeights);

        const stbProvider = new Filament.gltfio$StbProvider(engine);
        const ktx2Provider = new Filament.gltfio$Ktx2Provider(engine);

        resourceLoader.addStbProvider("image/jpeg", stbProvider);
        resourceLoader.addStbProvider("image/png", stbProvider);
        resourceLoader.addKtx2Provider("image/ktx2", ktx2Provider);

        const onComplete = () => {
            resourceLoader.asyncBeginLoad(asset);

            // NOTE: This decodes in the wasm layer instead of using Canvas2D, which allows Filament
            // to have more control (handling of alpha, srgb, etc) and improves parity with native
            // platforms. In the future we may wish to offload this to web workers.

            // Decode a single PNG or JPG every 30 milliseconds, or at the specified interval.
            const timer = setInterval(() => {
                resourceLoader.asyncUpdateLoad();
                const progress = resourceLoader.asyncGetLoadProgress();
                if (progress >= 1) {
                    clearInterval(timer);
                    resourceLoader.delete();
                    stbProvider.delete();
                    onDone();
                }
            }, interval);
        };

        // Download all non-texture resources and invoke the callback when done.
        if (bufferUris.length == 0) {
            onComplete();
        } else {
            Filament.fetch(bufferUris, onComplete, function(absoluteUri) {
                const buffer = getBufferDescriptor(absoluteUri);
                const relativeUri = absoluteToRelativeUri[absoluteUri];
                resourceLoader.addResourceData(relativeUri, buffer);
                buffer.delete();
                onFetched(relativeUri);
            });
        }

        // Begin downloading all texture resources, no completion callback necessary.
        Filament.fetch(textureUris, null, function(absoluteUri) {
            const buffer = getBufferDescriptor(absoluteUri);
            const relativeUri = absoluteToRelativeUri[absoluteUri];
            resourceLoader.addResourceData(relativeUri, buffer);
            buffer.delete();
            onFetched(relativeUri);
        });
    };

    Filament.gltfio$FilamentAsset.prototype.getEntities = function() {
        return Filament.vectorToArray(this._getEntities());
    };

    Filament.gltfio$FilamentAsset.prototype.getEntitiesByName = function(name) {
        return Filament.vectorToArray(this._getEntitiesByName(name));
    };

    Filament.gltfio$FilamentAsset.prototype.getEntitiesByPrefix = function(prefix) {
        return Filament.vectorToArray(this._getEntitiesByPrefix(prefix));
    };

    Filament.gltfio$FilamentAsset.prototype.getLightEntities = function() {
        return Filament.vectorToArray(this._getLightEntities());
    };

    Filament.gltfio$FilamentAsset.prototype.getRenderableEntities = function() {
        return Filament.vectorToArray(this._getRenderableEntities());
    };

    Filament.gltfio$FilamentAsset.prototype.getCameraEntities = function() {
        return Filament.vectorToArray(this._getCameraEntities());
    };

    Filament.gltfio$FilamentAsset.prototype.getResourceUris = function() {
        return Filament.vectorToArray(this._getResourceUris());
    }

    Filament.gltfio$FilamentAsset.prototype.getAssetInstances = function() {
        return Filament.vectorToArray(this._getAssetInstances());
    }

    Filament.gltfio$FilamentInstance.prototype.getMaterialVariantNames = function() {
        return Filament.vectorToArray(this._getMaterialVariantNames());
    }
};
