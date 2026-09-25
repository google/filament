# Filament samples

Each `.cpp` file in this directory is a standalone sample app built on `FilamentApp2`
(`libs/filamentapp`). The samples share their command-line handling through `common/arguments.cpp`,
so every sample accepts the common options listed by `--help`, such as `--api`, `--ibl`,
`--screenshot` and `--fixed-timestep`, in addition to its own.

The same sources are built for three targets:

- **Desktop:** Each sample is an executable that renders into an SDL window. It can also run with
  `--headless` for offscreen rendering or with `--remote` to be driven from a browser. See
  [Running the native samples](../BUILDING.md#running-the-native-samples).
- **Android:** The samples listed in `SampleDispatcher.cpp` are compiled into
  `libfilament-samples.a`.
- **WebAssembly:** A subset of the samples, currently `hellotriangle` and `gltf_viewer`, runs in a
  browser. This is described below.

## WebAssembly samples

A WASM sample runs the same `main()` as its desktop counterpart. `EmscriptenDisplayManager` stands in
for SDL: it renders into an HTML canvas and drives frames from `requestAnimationFrame`.

### Building

The WASM samples are excluded from the default WASM build, so building `filament.js` does not also
build them. After an initial `./build.sh -p wasm debug`, build them explicitly:

```shell
source $EMSDK/emsdk_env.sh
ninja -C out/cmake-wasm-debug filament-samples    # or a single sample, e.g. gltf_viewer
```

Each sample produces the following files in `out/cmake-wasm-debug/samples/`:

| File | Contents |
|---|---|
| `<sample>.html` | The page that loads and runs the sample, generated from `wasm_shell.html.in`. |
| `<sample>.js`, `<sample>.wasm` | The Emscripten module. |
| `<sample>.data` | Files preloaded into the virtual filesystem, if the sample has any. `gltf_viewer` preloads the ImGui font and the default IBL. |

### Running

The page cannot be opened directly from the filesystem, so serve the directory over HTTP:

```shell
emrun out/cmake-wasm-debug/samples --no_browser --port 8000
```

Then open, for example, `http://localhost:8000/gltf_viewer.html`.

### WebGL and WebGPU

The same samples run on either browser backend, selected as on desktop with `--api`:

- **WebGL 2** (`--api opengl`, or no `--api`) is always available. The display manager creates the
  WebGL context on the canvas before the engine starts.
- **WebGPU** (`--api webgpu`) requires a build configured with WebGPU support, `./build.sh -W -p wasm
  debug`, and a browser that implements WebGPU. Because requesting a device is asynchronous and
  `main()` is not, the shell requests the device first when argv selects WebGPU. It then hands the
  device to the module as `Module.preinitializedWebGPUDevice`, where `WebGPUPlatformWasm` picks it
  up. A page other than the shell has to do the same.

### The HTML shell (`wasm_shell.html.in`)

The module is built with `MODULARIZE=1` and `INVOKE_RUN=0`, so it neither loads nor runs on its own.
A page has to instantiate it and then call `main()`. `wasm_shell.html.in` is that page, written once
for all samples. `add_wasm_demo()` in `CMakeLists.txt` generates a copy per sample with
`configure_file()`, substituting the sample's name.

**Arguments.** The shell passes `main()` an argv taken from the query string, so a sample can be run
with different options without rebuilding:

- `?args=--api webgpu /models/foo.glb` splits one parameter on spaces.
- `?arg=/my%20models/foo.glb&arg=--split-view` passes one argument per parameter, for values that
  contain spaces.

Paths refer to the Emscripten virtual filesystem, whose root is also the asset root. The shell calls
`main()` as soon as the module is instantiated, and for WebGPU as soon as it has a device, so only
files preloaded at link time are available. A driver that needs to supply its own inputs has to use
its own page, which writes them through `Module.FS` before calling `callMain()`.

**Unsupported options.** `--headless` is rejected because the browser display manager always renders
to the canvas. `--remote` is rejected because the page is already running in a browser.
`--webgpu-backend` is ignored, since the browser supplies the WebGPU implementation. `--screenshot`
still works on both backends: it reads back the rendered frame and writes into the virtual
filesystem.

**Reproducibility.** Without `--fixed-timestep`, animation time follows `requestAnimationFrame` and
therefore the display's refresh rate. Pass `--fixed-timestep` for runs that are compared against
each other.

### Observing a run

The shell reports progress through `globalThis.filamentApp`, so a driver, such as a test harness or a
person in the devtools console, does not have to parse console output.

| Field | Meaning |
|---|---|
| `status` | Progresses through `loading`, `instantiated` and `running`, and ends in `exited` or in one of the failure states below. |
| `exited` | Becomes `true` when the run has finished and no further frame will be drawn. |
| `error` | Becomes a non-null string when the run fails. |
| `module` | The Emscripten module once instantiated. For example, `module.FS.readFile()` retrieves a screenshot. |

A driver should wait until either `exited` is `true` or `error` is non-null. The failure states are
the following:

- `instantiation-failed` means the module could not be loaded.
- `main-exited` means `main()` returned a non-zero status, typically because an argument was
  rejected. The reason is printed to the console.
- `main-failed` means `main()` threw.
- `aborted` means the runtime aborted, including after `main()` has returned.

The page also raises a `filament-app-exit` event on `globalThis` when the run ends. The event fires
after `FilamentApp2::shutdown()`, so any file the sample wrote, such as a screenshot, is complete by
then.
