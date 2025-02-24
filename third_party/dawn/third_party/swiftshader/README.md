# SwiftShader

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)

Introduction
------------

SwiftShader[^1] is a high-performance CPU-based implementation[^2] of the Vulkan[^3] 1.3 graphics API. Its goal is to provide hardware independence for advanced 3D graphics.

> NOTE: The [ANGLE](http://angleproject.org/) project can be used to achieve a layered implementation[^4] of OpenGL ES 3.1 (aka. "SwANGLE").

Building
--------

SwiftShader libraries can be built for Windows, Linux, and macOS.\
Android and Chrome (OS) build environments are also supported.

* **CMake**
\
  [Install CMake](https://cmake.org/download/) for Linux, macOS, or Windows and use either [the GUI](https://cmake.org/runningcmake/) or run the following terminal commands:
  ```
  cd build
  cmake ..
  cmake --build . --parallel

  ./vk-unittests
  ```
  Tip: Set the [CMAKE_BUILD_PARALLEL_LEVEL](https://cmake.org/cmake/help/latest/envvar/CMAKE_BUILD_PARALLEL_LEVEL.html) environment variable to control the level of parallelism.


* **Visual Studio**
\
  To build the Vulkan ICD library, use [Visual Studio 2019](https://visualstudio.microsoft.com/vs/community/) to open the project folder and wait for it to run CMake. Open the [CMake Targets View](https://docs.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=vs-2019#ide-integration) in the Solution Explorer and select the vk_swiftshader project to [build](https://docs.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=vs-2019#building-cmake-projects) it.


Usage
-----

The SwiftShader libraries act as drop-in replacements for graphics drivers.

On Windows, most applications can be made to use SwiftShader's DLLs by placing them in the same folder as the executable. On Linux, the `LD_LIBRARY_PATH` environment variable or `-rpath` linker option can be used to direct applications to search for shared libraries in the indicated directory first.

In general, Vulkan applications look for a shared library named `vulkan-1.dll` on Windows (`vulkan-1.so` on Linux). This 'loader' library then redirects API calls to the actual Installable Client Driver (ICD). SwiftShader's ICD is named `libvk_swiftshader.dll`, but it can be renamed to `vulkan-1.dll` to be loaded directly by the application. Alternatively, you can set the `VK_ICD_FILENAMES` environment variable to the path to `vk_swiftshader_icd.json` file that is generated under the build directory (e.g. `.\SwiftShader\build\Windows\vk_swiftshader_icd.json`). To learn more about how Vulkan loading works, read the [official documentation here](https://github.com/KhronosGroup/Vulkan-Loader/blob/master/loader/LoaderAndLayerInterface.md).

Contributing
------------

See [CONTRIBUTING.txt](CONTRIBUTING.txt) for important contributing requirements.

The canonical repository for SwiftShader is hosted at:
https://swiftshader.googlesource.com/SwiftShader.

All changes must be reviewed and approved in the [Gerrit](https://www.gerritcodereview.com/) review tool at:
https://swiftshader-review.googlesource.com. You must sign in to this site with a Google Account before changes can be uploaded.

Next, authenticate your account here:
https://swiftshader.googlesource.com/new-password (use the same e-mail address as the one configured as the [Git commit author](https://git-scm.com/book/en/v2/Getting-Started-First-Time-Git-Setup#_your_identity)).

All changes require a [Change-ID](https://gerrit-review.googlesource.com/Documentation/user-changeid.html) tag in the commit message. A [commit hook](https://git-scm.com/book/en/v2/Customizing-Git-Git-Hooks) may be used to add this tag automatically, and can be found at:
https://gerrit-review.googlesource.com/tools/hooks/commit-msg. You can execute `git clone https://swiftshader.googlesource.com/SwiftShader` and manually place the commit hook in `SwiftShader/.git/hooks/`, or to clone the repository and install the commit hook in one go:

    git clone https://swiftshader.googlesource.com/SwiftShader && (cd SwiftShader && git submodule update --init --recursive third_party/git-hooks && ./third_party/git-hooks/install_hooks.sh)

On Windows, this command line requires using the [Git Bash Shell](https://www.atlassian.com/git/tutorials/git-bash).

Changes are uploaded to Gerrit by executing:

    git push origin HEAD:refs/for/master

When ready, [add](https://gerrit-review.googlesource.com/Documentation/intro-user.html#adding-reviewers) a project [owner](OWNERS) as a reviewer on your change.

Some tests will automatically be run against the change. Notably, [presubmit.sh](tests/presubmit.sh) verifies the change has been formatted using [clang-format 11.0.1](tests/kokoro/gcp_ubuntu/check_style.sh). Most IDEs come with clang-format support, but may require upgrading/downgrading to the [clang-format version 11.0.0](https://github.com/llvm/llvm-project/releases/tag/llvmorg-11.0.0) *release* version (notably Chromium's buildtools has a clang-format binary which can be an in-between revision which produces different formatting results).

Testing
-------

SwiftShader's Vulkan implementation can be tested using the [dEQP](https://github.com/KhronosGroup/VK-GL-CTS) test suite.

See [docs/dEQP.md](docs/dEQP.md) for details.

Third-Party Dependencies
------------------------

The [third_party](third_party/) directory contains projects which originated outside of SwiftShader:

[subzero](third_party/subzero/) contains a fork of the [Subzero](https://chromium.googlesource.com/native_client/pnacl-subzero/) project. It originates from Google Chrome's (Portable) [Native Client](https://developer.chrome.com/native-client) project. The fork was made using [git-subtree](https://github.com/git/git/blob/master/contrib/subtree/git-subtree.txt) to include all of Subzero's history.

[llvm-subzero](third_party/llvm-subzero/) contains a minimized set of LLVM dependencies of the Subzero project.

[PowerVR_SDK](third_party/PowerVR_SDK/) contains a subset of the [PowerVR Graphics Native SDK](https://github.com/powervr-graphics/Native_SDK) for running several sample applications.

[googletest](third_party/googletest/) contains the [Google Test](https://github.com/google/googletest) project, as a Git submodule. It is used for running unit tests for Chromium, and Reactor unit tests. Run `git submodule update --init` to obtain/update the code. Any contributions should be made upstream.

Documentation
-------------

See [docs/Index.md](docs/Index.md).

Contact
-------

Public mailing list: [swiftshader@googlegroups.com](https://groups.google.com/forum/#!forum/swiftshader)

General bug tracker:  https://g.co/swiftshaderbugs \
Chrome specific bugs: https://bugs.chromium.org/p/swiftshader

License
-------

The SwiftShader project is licensed under the Apache License Version 2.0. You can find a copy of it in [LICENSE.txt](LICENSE.txt).

Files in the third_party folder are subject to their respective license.

Authors and Contributors
------------------------

The legal authors for copyright purposes are listed in [AUTHORS.txt](AUTHORS.txt).

[CONTRIBUTORS.txt](CONTRIBUTORS.txt) contains a list of names of individuals who have contributed to SwiftShader. If you're not on the list, but you've signed the [Google CLA](https://cla.developers.google.com/clas) and have contributed more than a formatting change, feel free to request to be added.

Notes and Disclaimers
---------------------

[^1]: This is not an official Google product.  
[^2]: Vulkan 1.3 conformance: https://www.khronos.org/conformance/adopters/conformant-products#submission_717  
[^3]: Trademarks are the property of their respective owners.  
[^4]: OpenGL ES 3.1 conformance: https://www.khronos.org/conformance/adopters/conformant-products/opengles#submission_906  