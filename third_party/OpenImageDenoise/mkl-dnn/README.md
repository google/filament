# Intel(R) Math Kernel Library for Deep Neural Networks (Intel(R) MKL-DNN)
![v0.90 beta](https://img.shields.io/badge/v0.90-beta-orange.svg)

> NOTE
>
> The master branch is now used to work on the upcoming Intel MKL-DNN v1.0 with
> incompatible changes to the v0.x. The changes are described in the following
> [RFC](https://github.com/intel/mkl-dnn/pull/384).
>
> For a limited time the team would maintain
> [0.x branch](https://github.com/intel/mkl-dnn/tree/mnt-v0),
> backporting fixes and some of the features from the mainline.

Intel(R) Math Kernel Library for Deep Neural Networks (Intel(R) MKL-DNN) is
an open-source performance library for deep-learning applications. The library
accelerates deep-learning applications and frameworks on Intel architecture.
Intel MKL-DNN contains vectorized and threaded building blocks that you can
use to implement deep neural networks (DNN) with C and C++ interfaces.

DNN functionality optimized for Intel architecture is also included in
[Intel Math Kernel Library (Intel MKL)](https://software.intel.com/en-us/mkl/features/deep-neural-networks).
The API in that implementation is not compatible with Intel MKL-DNN and does not
include certain new and experimental features.

This release contains performance-critical functions that improve performance of
the following deep learning topologies and variations of these:

| Application                               | Example topology
|:---                                       |:---
| Image recognition                         | AlexNet, VGG, GoogleNet, ResNet, MobileNet
| Image segmentation                        | FCN, SegNet, MaskRCNN, U-Net
| Volumetric segmentation                   | 3D-Unet
| Object detection                          | SSD, Faster R-CNN, Yolo
| Neural machine translation                | GNMT
| Speech recognition                        | DeepSpeech
| Adversarial networks                      | DCGAN, 3DGAN
| Reinforcement learning                    | A3C
| Text-to-speech                            | WaveNet

Intel MKL-DNN is used in the following software products:
* [Caffe\* Optimized for Intel Architecture](https://github.com/intel/caffe)
* [Chainer\*](https://chainer.org)
* [DeepBench](https://github.com/baidu-research/DeepBench)
* [PaddlePaddle\*](http://www.paddlepaddle.org)
* [PyTorch\*](https://pytorch.org/)
* [Tensorflow\*](https://www.tensorflow.org)
* [Microsoft\* Cognitive Toolkit (CNTK)](https://docs.microsoft.com/en-us/cognitive-toolkit)
* [Apache\* MXNet](https://mxnet.apache.org)
* [OpenVINO(TM) toolkit](https://01.org/openvinotoolkit)
* [Intel Nervana Graph](https://github.com/NervanaSystems/ngraph)
* [Menoh\*](https://github.com/pfnet-research/menoh)
* [DeepLearning4J\*](https://deeplearning4j.org)
* [BigDL](https://github.com/intel-analytics/BigDL)

## License
Intel MKL-DNN is licensed under
[Apache License Version 2.0](http://www.apache.org/licenses/LICENSE-2.0). This
software includes the following third-party components:
* [Xbyak](https://github.com/herumi/xbyak) distributed under [3-clause BSD licence](src/cpu/xbyak/COPYRIGHT)
* [gtest](https://github.com/google/googletest) distributed under [3-clause BSD license](tests/gtests/gtest/LICENSE)

## Documentation
* [Introduction](https://intel.github.io/mkl-dnn) explains the programming model
and basic concepts
* [Reference manual](https://intel.github.io/mkl-dnn/modules.html) provides
detailed functionality description
* [Examples](https://github.com/intel/mkl-dnn/tree/master/examples)
demonstrates use of C and C++ APIs in simple topologies
* [Tutorial](https://software.intel.com/en-us/articles/intel-mkl-dnn-part-1-library-overview-and-installation)
provides step-by-step installation instructions and an example walkthrough

## Support
Please submit your questions, feature requests, and bug reports on the
[GitHub issues](https://github.com/intel/mkl-dnn/issues) page.

**WARNING** The following functionality has preview status and might change
without prior notification in future releases:
* Threading Building Blocks (TBB) support

## How to Contribute
We welcome community contributions to Intel MKL-DNN. If you have an idea on how to improve the library:

* Share your proposal via
 [GitHub issues](https://github.com/intel/mkl-dnn/issues).
* Ensure you can build the product and run all the examples with your patch.
* In the case of a larger feature, create a test.
* Submit a [pull request](https://github.com/intel/mkl-dnn/pulls).

We will review your contribution and, if any additional fixes or modifications
are necessary, may provide feedback to guide you. When accepted, your pull
request will be merged to the repository.

## System Requirements
Intel MKL-DNN supports Intel 64 architecture and compatible architectures.
The library is optimized for the systems based on
* Intel Atom(R) processor with Intel SSE4.1 support
* 4th, 5th, 6th, 7th, and 8th generation Intel(R) Core(TM) processor
* Intel(R) Xeon(R) processor E5 v3 family (formerly Haswell)
* Intel Xeon processor E5 v4 family (formerly Broadwell)
* Intel Xeon Platinum processor family (formerly Skylake)
* Intel(R) Xeon Phi(TM) processor x200 product family (formerly Knights Landing)
* Intel Xeon Phi processor x205 product family (formerly Knights Mill)

and compatible processors.

The software dependencies are:
* [Cmake](https://cmake.org/download/) 2.8.0 or later
* [Doxygen](http://www.stack.nl/~dimitri/doxygen/download.html#srcbin) 1.8.5 or later
* C++ compiler with C++11 standard support
* Optional dependencies:
  * GNU\* OpenMP\*, LLVM OpenMP, or Intel OpenMP
  * Threading Building Blocks (TBB) 2017 or later
  * Intel MKL 2017 Update 1 or Intel MKL small libraries

> **Note**
> Building Intel MKL-DNN with optional dependencies may introduce additional
> runtime dependencies for the library. For details, refer to the corresponding
> software system requirements.

The software was validated on RedHat\* Enterprise Linux 7 with
* GNU Compiler Collection 4.8, 5.4, 6.1, 7.2, and 8.1
* Clang\* 3.8.0
* [Intel C/C++ Compiler](https://software.intel.com/en-us/intel-parallel-studio-xe)
  17.0, 18.0, and 19.0

on Windows Server\* 2012 R2 with
* Microsoft Visual C++ 14.0 (Visual Studio 2015 Update 3)
* [Intel C/C++ Compiler](https://software.intel.com/en-us/intel-parallel-studio-xe)
  17.0 and 19.0

on macOS\* 10.13 (High Sierra) with
* Apple LLVM version 9.2 (XCode 9.2)
* [Intel C/C++ Compiler](https://software.intel.com/en-us/intel-parallel-studio-xe)
  18.0 and 19.0

The implementation uses OpenMP 4.0 SIMD extensions. We recommend using the
Intel C++ Compiler for the best performance results.

## Installation

### Build from source

#### Download source code
Download [Intel MKL-DNN source code](https://github.com/intel/mkl-dnn/archive/master.zip)
or clone [the repository](https://github.com/intel/mkl-dnn.git) to your system.

```
git clone https://github.com/intel/mkl-dnn.git
```

#### Configure build
Intel MKL-DNN uses a CMake-based build system. You can use CMake options to control the build.
Along with the standard CMake options such as `CMAKE_INSTALL_PREFIX` and `CMAKE_BUILD_TYPE`,
you can pass Intel MKL-DNN specific options:

|Option                 | Possible Values (defaults in bold)   | Description
|:---                   |:---                                  | :---
|MKLDNN_LIBRARY_TYPE    | **SHARED**, STATIC                   | Defines the resulting library type
|MKLDNN_THREADING       | **OMP**, OMP:INTEL, OMP:COMP, TBB    | Defines the threading type
|MKLDNN_BUILD_EXAMPLES  | **ON**, OFF                          | Controls building the examples
|MKLDNN_BUILD_TESTS     | **ON**, OFF                          | Controls building the tests
|MKLDNN_ARCH_OPT_FLAGS  | *compiler flags*                     | Specifies compiler optimization flags (see warning note below)
|VTUNEROOT              | *path*                               | Enables integration with Intel(R) VTune(TM) Amplifier

> **WARNING**
>
> By default, Intel MKL-DNN is built specifically for the processor type of the
> compiling machine (for example, `-march=native` in the case of GCC). While this option
> gives better performance, the resulting library can be run only on systems
> that are instruction-set compatible with the compiling machine.
>
> Therefore, if Intel MKL-DNN is to be shipped to other platforms (for example, built by
> Linux distribution maintainers), consider setting `MKLDNN_ARCH_OPT_FLAGS` to `""`.

For more options and details, check [cmake/options.cmake](cmake/options.cmake).

##### Using Intel MKL (optional)
Intel MKL-DNN includes an optimized matrix-matrix multiplication (GEMM) implementation for modern platforms.
The library can also take advantage of GEMM functions from Intel MKL to improve performance with older
versions of compilers or on older platforms. This behavior is controlled by the `MKLDNN_USE_MKL` option.

|Option                 | Possible Values (defaults in bold)   | Description
|:---                   |:---                                  | :---
|MKLDNN_USE_MKL         | **DEF**, NONE, ML, FULL, FULL:STATIC | Defines the binary dependency on Intel MKL

The dynamic library with this functionality is included in the repository.
If you choose to build Intel MKL-DNN with the binary dependency, download the Intel MKL small
libraries using the provided script:

*Linux/macOS*
```
cd scripts && ./prepare_mkl.sh && cd ..
```

*Windows\**
```
cd scripts && call prepare_mkl.bat && cd ..
```

or manually from [GitHub release section](https://github.com/intel/mkl-dnn/releases),
and unpack it to the `external` directory in the repository root. Intel MKL-DNN
can also be built with full Intel MKL if the latter is installed on the system.
You might need to set the `MKLROOT` environment variable to the path where the full
Intel MKL is installed to help `cmake` locate the library.

> **Note**
>
> Using Intel MKL small libraries currently works only for Intel MKL-DNN built with
> OpenMP. Building with Intel TBB requires either the full Intel MKL library
> or a standalone build.
>
> Using Intel MKL or Intel MKL small libraries will introduce additional
> runtime dependencies. For additional information, refer to Intel MKL
> [system requirements](https://software.intel.com/en-us/articles/intel-math-kernel-library-intel-mkl-2019-system-requirements).

##### Threading
Intel MKL-DNN is parallelized and can use the OpenMP or TBB threading runtime. OpenMP threading is the default build mode
and is recommended for the best performance. TBB support is experimental. This behavior is controlled by the `MKLDNN_THREADING` option.

|Option                 | Possible Values (defaults in bold)   | Description
|:---                   |:---                                  | :---
|MKLDNN_THREADING       | **OMP**, OMP:INTEL, OMP:COMP, TBB    | Defines the threading type

##### OpenMP
Intel MKL-DNN can use Intel, GNU or CLANG OpenMP runtime. Because different OpenMP runtimes may not be binary compatible,
it's important to ensure that only one OpenMP runtime is used throughout the
application. Having more than one OpenMP runtime initialized may lead to
undefined behavior including incorrect results or crashes.

Intel MKL-DNN library built with the binary dependency will link against the Intel OpenMP
runtime included with the Intel MKL small libraries package. The Intel OpenMP runtime
is binary compatible with the GNU OpenMP and Clang OpenMP runtimes and is
recommended for the best performance results.

Intel MKL-DNN library built standalone will use the OpenMP runtime supplied by
the compiler, so as long as both the library and the application use the
same compiler, the correct OpenMP runtime will be used.

##### TBB
TBB support is experimental. Intel MKL-DNN has limited optimizations done for Intel TBB and has some functional
limitations if built with Intel TBB.

Functional limitations:
* Convolution with Winograd algorithm is not supported

Performance limitations (mostly less parallelism than in case of OpenMP):
* Batch normalization
* Convolution backward by weights
* mkldnn_sgemm

> **WARNING**
>
> If the library is built with the full Intel MKL, the user is expected to set
> the `MKL_THREADING_LAYER` environment variable to either `tbb` or `sequential` in order
> to force Intel MKL to use Intel TBB for parallelization or to be sequential,
> respectively. Without this setting, Intel MKL (RT library) tries
> to use OpenMP for parallelization by default.

#### Build on Linux/macOS
Ensure that all software dependencies are in place and have at least the minimal
supported version.

Configure CMake and create a makefile:

```
mkdir -p build && cd build && cmake $CMAKE_OPTIONS ..
```

Build the application:

```
make
```

The build can be validated with the unit-test suite:

```
ctest
```

The reference manual is provided inline and can also be generated in HTML format with Doxygen:

```
make doc
```

Documentation will reside in the `build/reference/html` folder.

Finally:

```
make install
```

will place the header files, libraries, and documentation in `/usr/local`. To change
the installation path, use the option `-DCMAKE_INSTALL_PREFIX=<prefix>` when invoking CMake.

#### Build on Windows
Ensure that all software dependencies are in place and have at least the minimal
supported version.

> **NOTE**
>
> Building Intel MKL-DNN from a terminal requires using either the Intel Parallel Studio command prompt
> or the Microsoft\* Visual Studio\* developer command prompt instead of the default Windows command prompt.
>
> The Intel(R) Parallel Studio command prompt is an item in the **Start** menu in the **Intel Parallel Studio
> \<version\>** folder that has a Windows Command Prompt icon and a name like **Compiler 18.0 Update 5…**.
>
> The default for building the project for the Intel C++ Compiler is to use the Intel
> Parallel Studio developer command prompt.

Configure CMake and create a Microsoft Visual Studio solution:

```
mkdir build & cd build && cmake -G "Visual Studio 15 2017 Win64" ..
```

For the solution to use Intel C++ Compiler:

```
cmake -G "Visual Studio 15 2017 Win64" -T "Intel C++ Compiler 18.0" ..
```

After you have built the initial project using CMake, you can then open the project with
Microsoft Visual Studio and build from there. You can also use msbuild command-line tool
to build from the command line:

```
msbuild "Intel(R) MKL-DNN.sln" /p:Configuration=Release [/t:rebuild] /m
```
where the optional argument `/t:rebuild` rebuilds the project.

The build can be validated with the unit-test suite:

```
ctest
```

## Linking Your Application

### Linux/macOS
Intel MKL-DNN includes several header files providing C and C++ APIs for
the functionality and one or several dynamic libraries depending on how
Intel MKL-DNN was built.

**Linux**

|File                   | Description
|:---                   |:---
|include/mkldnn.h       | C header
|include/mkldnn.hpp     | C++ header
|include/mkldnn_types.h | Auxiliary C header
|lib/libmkldnn.so       | Intel MKL-DNN dynamic library
|lib/libmkldnn.a        | Intel MKL-DNN static library (if built with `MKLDNN_LIBRARY_TYPE=STATIC`)
|lib/libiomp5.so        | Intel OpenMP\* runtime library (if built with `MKLDNN_USE_MKL=ML`)
|lib/libmklml_gnu.so    | Intel MKL small library for GNU OpenMP runtime (if built with `MKLDNN_USE_MKL=ML`)
|lib/libmklml_intel.so  | Intel MKL small library for Intel OpenMP runtime (if built with `MKLDNN_USE_MKL=ML`)

**macOS**

|File                     | Description
|:---                     |:---
|include/mkldnn.h         | C header
|include/mkldnn.hpp       | C++ header
|include/mkldnn_types.h   | Auxiliary C header
|lib/libmkldnn.dylib      | Intel MKL-DNN dynamic library
|lib/libmkldnn.a          | Intel MKL-DNN static library (if built with `MKLDNN_LIBRARY_TYPE=STATIC`)
|lib/libiomp5.dylib       | Intel OpenMP\* runtime library (if built with `MKLDNN_USE_MKL=ML`)
|lib/libmklml_gnu.dylib   | Intel MKL small library for GNU OpenMP runtime (if built with `MKLDNN_USE_MKL=ML`)
|lib/libmklml_intel.dylib | Intel MKL small library for Intel OpenMP runtime (if built with `MKLDNN_USE_MKL=ML`)

Linkline examples below assume that Intel MKL-DNN is installed in the directory
defined in the MKLDNNROOT environment variable.

```
g++ -std=c++11 -I${MKLDNNROOT}/include -L${MKLDNNROOT}/lib simple_net.cpp -lmkldnn
clang -std=c++11 -I${MKLDNNROOT}/include -L${MKLDNNROOT}/lib simple_net.cpp -lmkldnn
icpc -std=c++11 -I${MKLDNNROOT}/include -L${MKLDNNROOT}/lib simple_net.cpp -lmkldnn
```

> **WARNING**
>
> Using the GNU compiler with the `-fopenmp` and `-liomp5` options will link the
> application with both the Intel and GNU OpenMP runtime libraries. This will lead
> to undefined behavior in the application.

> **NOTE**
>
> Applications linked dynamically will resolve the dependencies at runtime. 
> Make sure that the dependencies are available in the standard locations
> defined by the operating system, in the locatons listed in `LD_LIBRARY_PATH` (Linux),
> `DYLD_LIBRARY_PATH` (macOS) environment variables, or `rpath` mechanism.

### Windows
Intel MKL-DNN includes several header files providing C and C++ APIs for
the functionality and one or several dynamic libraries depending on how
Intel MKL-DNN was built.

|File                   | Description
|:---                   |:---
|bin\libmkldnn.dll      | Intel MKL-DNN dynamic library
|bin\libiomp5.dll       | Intel OpenMP\* runtime library (if built with `MKLDNN_USE_MKL=ML`)
|bin\libmklml.dll       | Intel MKL small library (if built with `MKLDNN_USE_MKL=ML`)
|include\mkldnn.h       | C header
|include\mkldnn.hpp     | C++ header
|include\mkldnn_types.h | Auxiliary C header
|lib\libmkldnn.lib      | Intel MKL-DNN import library
|lib\libiomp5.lib       | Intel OpenMP\* runtime import library (if built with `MKLDNN_USE_MKL=ML`)
|lib\libmklml.lib       | Intel MKL small library import library (if built with `MKLDNN_USE_MKL=ML`)

To link the application from the command line, set up the `LIB` and `INCLUDE` environment variables to point to the locations of 
the Intel MKL-DNN headers and libraries. The Linkline examples below assume that Intel MKL-DNN is installed in the directory
defined in the MKLDNNROOT environment variable. 

```
set INCLUDE=%MKLDNNROOT%\include;%INCLUDE%
set LIB=%MKLDNNROOT%\lib;%LIB%
icl /Qstd=c++11 /qopenmp simple_net.cpp mkldnn.lib
cl simple_net.cpp mkldnn.lib
```

Refer to [Microsoft Visual Studio documentation](https://docs.microsoft.com/en-us/cpp/build/walkthrough-creating-and-using-a-dynamic-link-library-cpp?view=vs-2017)
on linking the application using MSVS solutions.

> **NOTE**
> Applications linked dynamically will resolve the dependencies at runtime.
> Make sure that the dependencies are available in the standard locations
> defined by the operating system or in the locatons listed in the `PATH` environment variable.

--------

[Legal Information](doc/legal_information.md)
