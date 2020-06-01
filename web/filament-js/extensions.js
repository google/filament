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

Filament.loadClassExtensions = function() {

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
        Filament.glOptions = options;
        Filament.glContext = ctx;

        // Enable all desired extensions by calling getExtension on each one.
        ctx.getExtension('WEBGL_compressed_texture_s3tc');
        ctx.getExtension('WEBGL_compressed_texture_astc');
        ctx.getExtension('WEBGL_compressed_texture_etc');

        // Register the GL context with emscripten and create the Engine.
        return Filament.Engine._create();
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

    /// createTextureFromKtx ::method:: Utility function that creates a [Texture] from a KTX file.
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer] with KTX file contents
    /// options ::argument:: Options dictionary.
    /// ::retval:: [Texture]
    Filament.Engine.prototype.createTextureFromKtx = function(buffer, options) {
        buffer = getBufferDescriptor(buffer);
        const result = Filament._createTextureFromKtx(buffer, this, options);
        buffer.delete();
        return result;
    };

    /// createIblFromKtx ::method:: Utility that creates an [IndirectLight] from a KTX file.
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer] with KTX file contents
    /// options ::argument:: Options dictionary.
    /// ::retval:: [IndirectLight]
    Filament.Engine.prototype.createIblFromKtx = function(buffer, options) {
        buffer = getBufferDescriptor(buffer);
        const result = Filament._createIblFromKtx(buffer, this, options);
        buffer.delete();
        return result;
    };

    /// createSkyFromKtx ::method:: Utility function that creates a [Skybox] from a KTX file.
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer] with KTX file contents
    /// options ::argument:: Options dictionary.
    /// ::retval:: [Skybox]
    Filament.Engine.prototype.createSkyFromKtx = function(buffer, options) {
        const skytex = this.createTextureFromKtx(buffer, options);
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
        const materials = new Filament.gltfio$UbershaderLoader(this);
        return new Filament.gltfio$AssetLoader(this, materials);
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
    /// overrides ::argument:: Dictionary with one or more of the following properties: \
    /// radius, power, bias, resolution, intensity, quality.
    Filament.View.prototype.setAmbientOcclusionOptions = function(overrides) {
        const options = {
            radius: 0.3,
            power: 1.0,
            bias: 0.0005,
            resolution: 0.5,
            intensity: 1.0,
            quality: Filament.View$QualityLevel.LOW
        };
        Object.assign(options, overrides);
        this._setAmbientOcclusionOptions(options);
    };

    /// setDepthOfFieldOptions ::method::
    /// overrides ::argument:: Dictionary with one or more of the following properties: \
    /// focusDistance, blurScale, maxApertureDiameter, enabled.
    Filament.View.prototype.setDepthOfFieldOptions = function(overrides) {
        const options = {
            focusDistance: 10.0,
            blurScale: 1.0,
            maxApertureDiameter: 0.01,
            enabled: false
        };
        Object.assign(options, overrides);
        this._setDepthOfFieldOptions(options);
    };

    /// setBloomOptions ::method::
    /// overrides ::argument:: Dictionary with one or more of the following properties: \
    /// dirtStrength, strength, resolution, anomorphism, levels, blendMode, threshold, enabled, dirt.
    Filament.View.prototype.setBloomOptions = function(overrides) {
        const options = {
            dirtStrength: 0.2,
            strength: 0.10,
            resolution: 360,
            anamorphism: 1.0,
            levels: 6,
            blendMode: Filament.View$BloomOptions$BloomMode.ADD,
            threshold: true,
            enabled: false,
            dirt: null
        };
        Object.assign(options, overrides);
        this._setBloomOptions(options);
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

    Filament.RenderableManager$Builder.prototype.build =
    Filament.LightManager$Builder.prototype.build =
        function(engine, entity) {
            const result = this._build(engine, entity);
            this.delete();
            return result;
        };

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

    Filament.KtxBundle.prototype.getBlob = function(index) {
        const blob = this._getBlob(index);
        const result = blob.getBytes();
        blob.delete();
        return result;
    }

    Filament.KtxBundle.prototype.getCubeBlob = function(miplevel) {
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

    Filament.SurfaceOrientation$Builder.prototype.build = function() {
        const result = this._build();
        this.delete();
        return result;
    };

    Filament.gltfio$AssetLoader.prototype.createAssetFromJson = function(buffer) {
        if ('string' == typeof buffer && buffer.endsWith('.glb')) {
            console.error('Please use createAssetFromBinary for glb files.');
        }
        buffer = getBufferDescriptor(buffer);
        const result = this._createAssetFromJson(buffer);
        buffer.delete();
        return result;
    };

    Filament.gltfio$AssetLoader.prototype.createAssetFromBinary = function(buffer) {
        if ('string' == typeof buffer && buffer.endsWith('.gltf')) {
            console.error('Please use createAssetFromJson for gltf files.');
        }
        buffer = getBufferDescriptor(buffer);
        const result = this._createAssetFromBinary(buffer);
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
    Filament.gltfio$FilamentAsset.prototype.loadResources = function(onDone, onFetched, basePath,
            asyncInterval) {
        const asset = this;
        const engine = this.getEngine();
        const names = this.getResourceUris();
        const interval = asyncInterval || 30;

        basePath = basePath || document.location;
        onFetched = onFetched || ((name) => {});
        onDone = onDone || (() => {});

        // Construct the set of URI strings to fetch.
        const urlset = new Set();
        const urlToName = {};
        for (let i = 0; i < names.size(); i++) {
            const name = names.get(i);
            if (name) {
                const url = '' + new URL(name, basePath);
                urlToName[url] = name;
                urlset.add(url);
            }
        }

        // Construct a resource loader and start decoding after all textures are fetched.
        const resourceLoader = new Filament.gltfio$ResourceLoader(engine, false, false);
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
                    onDone();
                }
            }, interval);
        };

        if (urlset.size == 0) {
            onComplete();
            return;
        }

        // Begin downloading all external resources.
        Filament.fetch(Array.from(urlset), onComplete, function(url) {
            const buffer = getBufferDescriptor(url);
            const name = urlToName[url];
            resourceLoader.addResourceData(name, buffer);
            buffer.delete();
            onFetched(name);
        });
    };
};
