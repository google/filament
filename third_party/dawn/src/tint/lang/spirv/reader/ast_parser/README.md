# SPIR-V Reader

This component translates SPIR-V written for Vulkan into the Tint AST.

The SPIR-V reader entry point is `tint::spirv::reader::Read()`, which
performs the translation of SPIR-V to a `tint::Program`.

It's usable from the Tint command line:

    # Translate SPIR-V into WGSL.
    tint --format wgsl a.spv

## Supported dialects

The SPIR-V module must pass validation for the Vulkan 1.1 environment in SPIRV-Tools.
In particular, SPIR-V 1.4 and later are not supported.

For example, the equivalent of the following must pass:

    spirv-val --target-env vulkan1.1 a.spv

Additionally, the reader imposes additional constraints based on:

* The features supported by WGSL. Some Vulkan features might not be supportable because
   WebGPU must be portable to other graphics APIs.
* Limitations of the reader itself. These might be relaxed in the future with extra
   engineering work.

## Feedback

Please file issues at https://crbug.com/tint, and apply label `SpirvReader`.

Outstanding issues can be found by using the `SpirvReader` label in the Chromium project's
bug tracker: https://bugs.chromium.org/p/tint/issues/list?q=label:SpirvReader
