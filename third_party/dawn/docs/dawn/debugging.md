# Debugging Dawn

## Toggles
There are various debug-related Toggles that can help diagnose issues. Useful debug toggles:
 - `dump_shaders`: Log input WGSL shaders and translated backend shaders (MSL/ HLSL/DXBC/DXIL / SPIR-V).
 - `disable_symbol_renaming`: As much as possible, disable renaming of symbols (variables, function names, etc.). This can make dumped shaders more readable.
 - `emit_hlsl_debug_symbols`: Sets the D3DCOMPILE_SKIP_OPTIMIZATION and D3DCOMPILE_DEBUG compilation flags when compiling HLSL code.
 - `use_user_defined_labels_in_backend`: Forward object labels to the backend so that they can be seen in native debugging tools like RenderDoc, PIX, or Mac Instruments.

Toggles may be enabled/disabled in different ways.

- **In code:**

  Use extension struct `DawnTogglesDescriptor` chained on `DeviceDescriptor`.

  For example:
  ```c++
  const char* const enabledToggles[] = {"dump_shaders", "disable_symbol_renaming"};

  wgpu::DawnTogglesDescriptor deviceTogglesDesc;
  deviceTogglesDesc.enabledToggles = enabledToggles;
  deviceTogglesDesc.enabledTogglesCount = 2;

  wgpu::DeviceDescriptor deviceDescriptor;
  deviceDescriptor.nextInChain = &deviceTogglesDesc;
  ```

- **Command-line for Chrome**

  Run Chrome with command line flags`--enable-dawn-features` and/or `--disable-dawn-features` to force enable/disable toggles. Toggles should be comma-delimited.

  For example:
  `--enable-dawn-features=dump_shaders,disable_symbol_renaming`

- **Command-line for dawn_end2end_tests/dawn_unittests**

  Run Dawn test binaries with command line flags`--enable-toggles` and/or `--disable-toggles` to force enable/disable toggles. Toggles should be comma-delimited.

  For example:
  `dawn_end2end_tests --enable-toggles=dump_shaders,disable_symbol_renaming`

## Environment Variables

 - `DAWN_DEBUG_BREAK_ON_ERROR`

    Errors in WebGPU are reported asynchronously which may make debugging difficult because at the time an error is reported, you can't easily create a breakpoint to inspect the callstack in your application.

    Setting `DAWN_DEBUG_BREAK_ON_ERROR` to a non-empty, non-zero value will execute a debug breakpoint
    instruction ([`dawn::Breakpoint()`](https://source.chromium.org/chromium/chromium/src/+/main:third_party/dawn/src/dawn/common/Assert.cpp?q=dawn::Breakpoint)) as soon as any type of error is generated.
