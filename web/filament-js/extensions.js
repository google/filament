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
            depth: false,
            alpha: false
        };
        options = Object.assign(defaults, options);

        // Create the WebGL 2.0 context and register it with emscripten.
        const ctx = canvas.getContext("webgl2", options);
        const handle = GL.registerContext(ctx, options);
        GL.makeContextCurrent(handle);

        // Enable all desired extensions by calling getExtension on each one.
        ctx.getExtension('WEBGL_compressed_texture_s3tc');
        ctx.getExtension('WEBGL_compressed_texture_astc');
        ctx.getExtension('WEBGL_compressed_texture_etc');

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
        const result = Filament._createTextureFromPng(buffer, this, options);
        buffer.delete();
        return result;
    };

    /// createTextureFromJpeg ::method:: Creates a 2D [Texture] from a JPEG image.
    /// image ::argument:: asset string or DOM Image that has already been loaded
    /// options ::argument:: JavaScript object with optional `srgb` and `nomips` keys.
    /// ::retval:: [Texture]
    Filament.Engine.prototype.createTextureFromJpeg = function(image, options) {
        options = options || {};
        if ('string' == typeof image || image instanceof String) {
            image = Filament.assets[image];
        }
        return Filament._createTextureFromJpeg(image, this, options);
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

    /// createAssetLoader ::static method::
    /// engine ::argument:: an instance of [Engine]
    /// ::retval:: an instance of [AssetLoader]
    /// Clients should create only one asset loader for the lifetime of their app, this prevents
    /// memory leaks and duplication of Material objects.
    Filament.Engine.prototype.createAssetLoader = function() {
        const materials = new Filament.gltfio$UbershaderLoader(this);
        return new Filament.gltfio$AssetLoader(this, materials);
    };

    /// VertexBuffer ::core class::

    /// setBufferAt ::method::
    /// engine ::argument:: [Engine]
    /// bufferIndex ::argument:: non-negative integer
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer]
    Filament.VertexBuffer.prototype.setBufferAt = function(engine, bufferIndex, buffer) {
        buffer = getBufferDescriptor(buffer);
        this._setBufferAt(engine, bufferIndex, buffer);
        buffer.delete();
    };

    /// IndexBuffer ::core class::

    /// setBuffer ::method::
    /// engine ::argument:: [Engine]
    /// buffer ::argument:: asset string, or Uint8Array, or [Buffer]
    Filament.IndexBuffer.prototype.setBuffer = function(engine, buffer) {
        buffer = getBufferDescriptor(buffer);
        this._setBuffer(engine, buffer);
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

    // See the C++ documentation for ResourceLoader and AssetLoader. The JavaScript API differs in
    // that it takes two optional callbacks:
    //
    // - onDone is called after all resources have been downloaded, but before the
    //   asset has been finalized. The onDone callback is passed the finalize function.
    //
    // - onFetched is called after each resource has finished downloading.
    //
    // "Finalization" refers to decoding texture data, converting the format of the
    // vertex data if needed, and potentially computing tangents.
    //
    // Takes an optional base path for resolving the URI strings in the glTF file, which is
    // typically the path to the parent glTF file. The given base path cannot itself be a relative
    // URL, but clients can do the following to resolve a relative URL:
    //    const basePath = '' + new URL(myRelativeUrl, document.location);
    // If the given base path is null, document.location is used as the base.
    Filament.gltfio$FilamentAsset.prototype.loadResources = function(onDone, onFetched, basePath) {
        const asset = this;
        const engine = this.getEngine();
        const names = this.getResourceUris();
        const urlset = new Set();
        const urlToName = {};

        basePath = basePath || document.location;

        for (var i = 0; i < names.size(); i++) {
            const name = names.get(i);
            if (name) {
                const url = '' + new URL(name, basePath);
                urlToName[url] = name;
                urlset.add(url);
            }
        }
        const resourceLoader = new Filament.gltfio$ResourceLoader(engine);

        const onComplete = function() {
            const finalize = function() {
                resourceLoader.loadResources(asset);

                // The buffer data won't get sent to the GPU until the next call to
                // "renderer.render()", so wait two frames before freeing the CPU-side data.
                window.requestAnimationFrame(function() {
                    window.requestAnimationFrame(function() {
                        resourceLoader.delete();
                    });
                });
            };
            if (onDone) {
                onDone(finalize);
            } else {
                finalize();
            }
        };

        if (urlset.size == 0) {
            onComplete();
            return;
        }

        Filament.fetch(Array.from(urlset), onComplete, function(url) {
            const buffer = getBufferDescriptor(url);
            const name = urlToName[url];
            resourceLoader.addResourceData(name, buffer);
            buffer.delete();
            if (onFetched) {
                onFetched(name);
            }
        });
    };
};
