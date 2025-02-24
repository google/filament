WebGPU tests that require manual intervention.

Many of these test may be HTML pages rather than using the harness.

Add informal notes here on possible stress tests.

- Suspending or hibernating the machine.
- Manually crashing or relaunching the browser's GPU process.
- Triggering a GPU driver reset (TDR).
- Forcibly or gracefully unplugging an external GPU.
- Forcibly switching between GPUs using OS/driver settings.
- Backgrounding the browser (on mobile OSes).
- Moving windows between displays attached to different hardware adapters.
- Moving windows between displays with different color properties (HDR/WCG).
- Unplugging a laptop.
- Switching between canvas and XR device output.

TODO: look at dEQP (OpenGL ES and Vulkan) and WebGL for inspiration here.
