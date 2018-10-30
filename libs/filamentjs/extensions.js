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
function getBufferDescriptor(buffer) {
  if ('string' == typeof buffer || buffer instanceof String) {
    buffer = Filament.assets[buffer];
  }
  if (buffer instanceof Uint8Array) {
    buffer = Filament.Buffer(buffer);
  }
  return buffer;
}

 Filament.loadClassExtensions = function() {

  /// Engine ::core class::

  /// create ::static method::
  /// canvas ::argument::
  /// options ::argument::
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
    Filament.createContext(canvas, true, true, options);
    return Filament.Engine._create();
  };

  /// createMaterial ::method::
  /// package ::argument:: asset string, or Uint8Array, or [Buffer] with filamat contents
  /// ::retval:: an instance of [createMaterial]
  Filament.Engine.prototype.createMaterial = function(buffer) {
    return this._createMaterial(getBufferDescriptor(buffer));
  };

  /// createTextureFromKtx ::method:: Utility function that creates a [Texture] from a KTX file.
  /// buffer ::argument:: asset string, or Uint8Array, or [Buffer] with KTX file contents
  /// options ::argument:: Options dictionary. For now, the `rgbm` boolean is the only option.
  /// ::retval:: [Texture]
  Filament.Engine.prototype.createTextureFromKtx = function(buffer, options) {
    return Filament._createTextureFromKtx(getBufferDescriptor(buffer), this, options);
  };

  /// createIblFromKtx ::method:: Utility function that creates an [IndirectLight] from a KTX file.
  /// buffer ::argument:: asset string, or Uint8Array, or [Buffer] with KTX file contents
  /// options ::argument:: Options dictionary. For now, the `rgbm` boolean is the only option.
  /// ::retval:: [IndirectLight]
  Filament.Engine.prototype.createIblFromKtx = function(buffer, options) {
    return Filament._createIblFromKtx(getBufferDescriptor(buffer), this, options);
  }

  /// createSkyFromKtx ::method:: Utility function that creates a [Skybox] from a KTX file.
  /// buffer ::argument:: asset string, or Uint8Array, or [Buffer] with KTX file contents
  /// options ::argument:: Options dictionary. For now, the `rgbm` boolean is the only option.
  /// ::retval:: [Skybox]
  Filament.Engine.prototype.createSkyFromKtx = function(buffer, options) {
    options = options || {'rgbm': true};
    const skytex = this.createTextureFromKtx(buffer, options);
    return Filament.Skybox.Builder().environment(skytex).build(this);
  };

  /// createTextureFromPng ::method:: Creates a 2D [Texture] from the raw contents of a PNG file.
  /// buffer ::argument:: asset string, or Uint8Array, or [Buffer] with PNG file contents
  /// options ::argument:: JavaScript object with optional `rgbm`, `noalpha`, and `nomips` keys.
  /// ::retval:: [Texture]
  Filament.Engine.prototype.createTextureFromPng = function(buffer, options) {
    return Filament._createTextureFromPng(getBufferDescriptor(buffer), this, options);
  }

  /// loadFilamesh ::method:: Consumes the contents of a filamesh file and creates a renderable.
  /// buffer ::argument:: asset string, or Uint8Array, or [Buffer] with filamesh contents
  /// definstance ::argument:: Optional default [MaterialInstance]
  /// matinstances ::argument:: Optional object that gets populated with name => [MaterialInstance]
  /// ::retval:: JavaScript object with keys `renderable`, `vertexBuffer`, and `indexBuffer`. \
  /// These are of type [Entity], [VertexBuffer], and [IndexBuffer].
  Filament.Engine.prototype.loadFilamesh = function(buffer, definstance, matinstances) {
    return Filament._loadFilamesh(this, getBufferDescriptor(buffer), definstance, matinstances);
  }

};
