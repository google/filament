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

Filament.onReadyListeners = [];

Filament.isReady = false;

/// init ::function:: Downloads assets, loads the Filament module, and invokes a callback when done.
///
/// All JavaScript clients must call the init function, passing in a list of asset URL's and a
/// callback. This callback gets invoked only after all assets have been downloaded and the Filament
/// WebAssembly module has been loaded. Clients should only pass asset URL's that absolutely must
/// be ready at initialization time.
///
/// When the callback is called, each downloaded asset is available in the `Filament.assets` global
/// object, which contains a mapping from URL's to Uint8Array objects.
///
/// assets ::argument:: Array of strings containing URL's of required assets.
/// onready ::argument:: callback that gets invoked after all assets have been downloaded and the \
/// Filament WebAssembly module has been loaded.
Filament.init = (assets, onready) => {
    if (onready) {
        Filament.onReadyListeners.push(onready);
    }
    if (Filament.initialized) {
        console.assert(!assets || assets.length == 0, "Assets can be specified only with the first call to init.");
        return;
    };
    Filament.initialized = true;

    Filament.assets = {};

    // Usage of glmatrix is optional. If it exists, then go ahead and augment it with some
    // useful math functions.
    if (typeof glMatrix !== 'undefined') {
        Filament.loadMathExtensions();
    }

    // One task for compiling & loading the wasm file, plus one task for each asset.
    let remainingTasks = 1 + assets.length;
    const taskFinished = () => {
        if (--remainingTasks == 0) {
            for (const callback of Filament.onReadyListeners) {
                callback();
            }
            Filament.isReady = true;
        }
    };

    // Issue a fetch for each asset.
    Filament.fetch(assets, null, taskFinished);

    // Emscripten creates a global function called "Filament" that returns a promise that
    // resolves to a module. Here we replace the function with the module. Note that our
    // TypeScript bindings assume that Filament is a namespace, not a function.
    Filament().then(module => {

        // Merge our extension functions into the emscripten module, not the other
        // way around, because Emscripten potentially replaces the HEAPU8 views in
        // the original module object (e.g. if it needs to grow the heap).
        Filament = Object.assign(module, Filament);

        // At this point, emscripten has finished compiling and instancing the WebAssembly module.
        // The JS classes that correspond to core Filament classes (e.g., Engine) are not guaranteed
        // to exist until now.

        Filament.loadClassExtensions();
        taskFinished();
    });
};

Filament.clearAssetCache = () => {
    for (const key in Filament.assets) delete Filament.assets[key];
};

/// fetch ::function:: Downloads assets and invokes a callback when done.
///
/// This utility consumes an array of URI strings and invokes callbacks after each asset is
/// downloaded. Additionally, each downloaded asset becomes available in the `Filament.assets`
/// global object, which is a mapping from URI strings to `Uint8Array`. If desired, clients can
/// pre-populate entries in `Filament.assets` to circumvent HTTP requests (this should be done after
/// calling `Filament.init`).
///
/// This function is used internally by `Filament.init` and `gltfio$FilamentAsset.loadResources`.
///
/// assets ::argument:: Array of strings containing URL's of required assets.
/// onDone ::argument:: callback that gets invoked after all assets have been downloaded.
/// onFetched ::argument:: optional callback that's invoked after each asset is downloaded.
Filament.fetch = (assets, onDone, onFetched) => {
    let remainingAssets = assets.length;
    assets.forEach(name => {

        // Check if a buffer already exists in case the client wishes
        // to provide its own data rather than using a HTTP request.
        if (Filament.assets[name]) {
            if (onFetched) {
                onFetched(name);
            }
            if (--remainingAssets === 0 && onDone) {
                onDone();
            }
        } else {
            fetch(name).then(response => {
                if (!response.ok) {
                    throw new Error(name);
                }
                return response.arrayBuffer();
            }).then(arrayBuffer => {
                Filament.assets[name] = new Uint8Array(arrayBuffer);
                if (onFetched) {
                    onFetched(name);
                }
                if (--remainingAssets === 0 && onDone) {
                    onDone();
                }
            });
        }
    });
};
