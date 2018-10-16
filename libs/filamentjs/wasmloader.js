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

 /*
 * WHY DOES THIS FILE EXIST?
 * -------------------------
 *
 * For each generated wasm file, emscripten generates a corollary JS file that is used to load the
 * wasm and set up its imports / exports. This file is a hand-authored addendum to the JS file that
 * emscripten generates, and we use it to store JavaScript-side utilities such as the asset
 * downloader and BufferDescriptor wrapper.
 *
 * Note that we should avoid certain modern JavaScript features in this file (e.g., for-of loops)
 * because emscripten uses a fairly rusty minifier.
 */

// Keep a counter of remaining asynchronous tasks (e.g., downloading assets) that must occur before
// allowing the app to initialize. As soon as the counter hits zero, we know it is time to call the
// app's onready callback. There is always at least 1 initialization task for loading the WASM
// module.
Filament.remainingInitializationTasks = 1;

// ENTRY POINT:
// Filament.init(assets, onready)
//
// All JavaScript clients must call the init function, passing in a list of asset URL's and a
// callback. This callback gets invoked only after all assets have been downloaded and the Filament
// WebAssembly module has been loaded. Clients should only pass asset URL's that absolutely must
// be ready at initialization time.
//
// When the callback is called, each downloaded asset is available in the form of Uint8Array in the
// "Filament.assets" object. The key is the URL and the value is the Uint8Array.
//
Filament.init = function(assets, onready) {
    Filament.onReady = onready;
    Filament.remainingInitializationTasks += assets.length;
    Filament.assets = {};
    assets.forEach(function(name) {
        fetch(name).then(function(response) {
            if (!response.ok) {
                throw new Error(name);
            }
            return response.arrayBuffer();
        }).then(function(arrayBuffer) {
            Filament.assets[name] = new Uint8Array(arrayBuffer);
            if (--Filament.remainingInitializationTasks == 0) {
                Filament.onReady();
            }
        });
    });
};

// This wrapper makes it easy for JavaScript clients to pass large swaths of data to Filament. It
// copies the contents of the given typed array into the WASM heap, then returns a low-level buffer
// descriptor object.
Filament.Buffer = function(typedarray) {
    console.assert(typedarray.buffer instanceof ArrayBuffer);
    console.assert(typedarray.byteLength > 0);
    // If the given array was taken from the WASM heap, then we need to create a copy because its
    // pointer will become invalidated after we instantiate the driver$PixelBufferDescriptor.
    if (Filament.HEAPU32.buffer == typedarray.buffer) {
        typedarray = new Uint8Array(typedarray);
    }
    const ta = typedarray;
    const bd = new Filament.driver$BufferDescriptor(ta);
    const uint8array = new Uint8Array(ta.buffer, ta.byteOffset, ta.byteLength);
    bd.getBytes().set(uint8array);
    return bd;
};

Filament.PixelBuffer = function(typedarray, format, datatype) {
    console.assert(typedarray.buffer instanceof ArrayBuffer);
    console.assert(typedarray.byteLength > 0);
    // If the given array was taken from the WASM heap, then we need to create a copy because its
    // pointer will become invalidated after we instantiate the driver$PixelBufferDescriptor.
    if (Filament.HEAPU32.buffer == typedarray.buffer) {
        typedarray = new Uint8Array(typedarray);
    }
    const ta = typedarray;
    const bd = new Filament.driver$PixelBufferDescriptor(ta, format, datatype);
    const uint8array = new Uint8Array(ta.buffer, ta.byteOffset, ta.byteLength);
    bd.getBytes().set(uint8array);
    return bd;
};

// The postRun method is called by emscripten after it finishes compiling and instancing the
// WebAssembly module. The JS classes that correspond to core Filament classes (e.g., Engine)
// are not guaranteed to exist until this function is called.
Filament.postRun = function() {
    Filament.Engine.create = function(canvas) {
        Filament.createContext(canvas, true, true, {
            majorVersion: 2,
            minorVersion: 0,
            antialias: false,
            depth: false,
            alpha: false
        });
        return Filament.Engine._create();
    };
    if (--Filament.remainingInitializationTasks == 0 && Filament.onReady) {
        Filament.onReady();
    }
};
