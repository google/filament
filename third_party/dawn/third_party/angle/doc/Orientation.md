# ANGLE Orientation

A basic guide to get up and running fixing bugs and performance issues in ANGLE.

## First ANGLE Compile

### Windows

- Download and install
  [Visual Studio Community](https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx).
  Installing takes some time.

- Take the time to register a Microsoft account, otherwise you'll get nagged to death.

- Download and install Chromium's
  [depot_tools](http://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up)
  for building ANGLE.

- Add the `depot_tools` dir to your system path. Open start menu, type "edit environment variables",
  add it to PATH.

- (recommended) Download and install [Git for Windows](http://gitforwindows.org/).

- Open Git bash, head to C:/src and follow the steps on
  [the ANGLE wiki](https://chromium.googlesource.com/angle/angle/+/main/doc/DevSetup.md#Development-setup-Getting-the-source)
  to set up the ANGLE solution for the first time.

- If you follow the [ANGLE wiki VS solution building and debugging guide](https://chromium.googlesource.com/angle/angle/+/main/doc/DevSetup.md#building-and-debugging-with-visual-studio), the VS solution will be in `c:/src/angle/out/Debug/angle-debug.sln`. Open and let the installation
  finish.  **Important**: set indent style to spaces, not tabs!

- Building should work at this point!

- Try running `angle_end2end_tests`, `angle_unittests` or a sample program.

- Useful VS extensions:

  1. [Build Only Startup Project](https://marketplace.visualstudio.com/items?itemName=SenHarada.BuildOnlyStartupProject)
  2. [SwitchStartupProject](https://marketplace.visualstudio.com/items?itemName=vs-publisher-141975.SwitchStartupProject)
  3. [Smart CommandLine Arguments](https://www.visualstudiogallery.msdn.microsoft.com/535f79b1-fbe0-4b0a-a346-8cdf271ea071)

### Linux

- Download and install Chromium's
  [depot_tools](http://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up)
  for building ANGLE.

- Ensure you add `depot_tools` to your bashrc as in the wiki link above.

- Follow the steps on
  [the ANGLE wiki](https://chromium.googlesource.com/angle/angle/+/main/doc/DevSetup.md#Development-setup-Getting-the-source)
  to setup ANGLE's build.

- Building should work at this point! Follow the steps on the Wiki.

- Try running `angle_end2end_tests`, `angle_unittests` or a sample program.

## Setting up the [drawElements testing suite](http://go/dEQP)

- [Cherry](https://sites.google.com/a/google.com/deqp/cherry) is the UI for viewing test results.
  ANGLE checks out a copy in `<angledir>/third_party/cherry`.

- Follow the instructions in the
  [installation README](https://android.googlesource.com/platform/external/cherry/+/refs/heads/main/README)
  to get it running. On Windows, use 64-bit.

- Read up on testing with
  [dEQP on the ANGLE Wiki](https://chromium.googlesource.com/angle/angle/+/main/doc/dEQP.md).

- Try running `angle_deqp_gles2_tests` with the flag
  `--gtest_filter=dEQP-GLES2.functional.negative_api.*` and load a test report in Cherry.

- To use Cherry, browse to [http://localhost:8080/#/results](http://localhost:8080/#/results) and
  click '**Import existing batch**', loading `TestResults.qpa`.  Look for the qpa file in the
  current working directory, or `<angledir>/src/tests` if you ran the tests from Visual Studio.

- Note: we only use Cherry for viewing test output, not running the tests. On start, you may see
  some runtime messages about unable to load case lists. These are safe to ignore. If you didn't
  load the results URL directly, click the "**Results**" tab to find the Import button.

## Profiling

### With Visual Studio

- In Visual Studio 2017, look under Debug/Profiler/Performance Explorer/New Performance Session.
  Right-click "Targets" and add `angle_perftests` as a Target Project.

- Run `angle_perftests` with the flag `--gtest_filter=DrawCallPerfBenchmark.Run/d3d11_null` for
  D3D11, `.../d3d9_null` for D3D9, `.../gl_null` for OpenGL and `.../vulkan_null` for Vulkan.

- Make sure you close all open instances of Chrome, they use a lot of background CPU and GPU. In
  fact, close every process and application you can.

### Profiling with Visual Studio + Chrome

- Install [Chrome Canary](https://www.google.com/chrome/browser/canary.html).

- Canary's install dir is usually `%APPDATA%/Local/Google/Chrome SxS/Application`

- Build ANGLE x64, Release, and run 'python scripts/update_chrome_angle.py' to replace Canary's
  ANGLE with your custom ANGLE. (Note: Canary must be closed)

- Start Canary with `--gpu-startup-dialog --disable-gpu-sandbox`, wait for the dialog.

- In Visual Studio, under Debug/Profiler, choose attach to process.

- Attach to the Chrome GPU process, then immediately pause profiling.

- **IMPORTANT:** Verify ANGLE details are correct in `about:gpu`.

- In Canary, start your benchmark, then resume profiling, and exit when done. The report will load
  automatically.

## Bookmark the latest Khronos specs

- [The GLES 2.0 Spec](https://www.khronos.org/registry/OpenGL/specs/es/2.0/es_full_spec_2.0.pdf)

- [The GLES 3.0 Spec](https://www.khronos.org/registry/OpenGL/specs/es/3.0/es_spec_3.0.pdf)

- [The GLES 3.1 Spec](https://www.khronos.org/registry/gles/specs/3.1/es_spec_3.1.pdf)

- [The GLES Shading Language 1.00 Spec](https://www.khronos.org/files/opengles_shading_language.pdf)

- [The GLES Shading Language 3.00 Spec](https://www.khronos.org/registry/gles/specs/3.0/GLSL_ES_Specification_3.00.4.pdf)

- [The WebGL Specs](https://www.khronos.org/registry/webgl/specs/latest/)

- [A modern desktop OpenGL Spec](https://www.opengl.org/registry/doc/glspec45.core.pdf)
  (for reference)

- [The Vulkan Spec](https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/html/vkspec.html)

These specs can be found in the [OpenGL Registry](https://github.com/KhronosGroup/OpenGL-Registry)
and the [Vulkan Docs](https://github.com/KhronosGroup/Vulkan-Docs) repositories as well.

## Join Groups and Chats

- Join the `#angle` channel in `chromium.slack.com`.

### For Googlers

- Join angle-team@ for access to many important emails and shared documents.

- We have a Hangouts Chat channel. Ask for an invite.
