# Standalone glslang validator

This folder contains a standalone glslang validator binary. We use this binary
for offline compilation of internal Vulkan shaders. See [the internal shader
docs](../../src/libANGLE/renderer/vulkan/shaders/README.md) for more info on
offline shader compilation.

Use the script [`update_glslang_binary.py`](update_glslang_binary.py) to update
the versions of the validator in cloud storage. It must be run on Linux or
Windows. It will update the SHA for your platform. After running the script run
`git commit` and then `git cl upload` to code review using the normal review
process. Note that if the version of glslang has been updated you will also want
to run [`scripts/run_code_generation.py`](../../scripts/run_code_generation.py)
to update the compiled shader binaries.

Please update both Windows and Linux binaries at the same time. Use two CLs. One
for each platform. Note that we don't currently support Mac on Vulkan. If we do
we should add a glslang download for Mac as well.

Contact syoussefi for any help with the validator or updating the binaries.
