Subzero Documentation
=====================

Subzero is a JIT compiler used as a back-end for [Reactor](Reactor.md). It originates from Chrome's [Portable Native Client](https://developer.chrome.com/native-client) project. Its authoritative repository is at [https://chromium.googlesource.com/native_client/pnacl-subzero/](https://chromium.googlesource.com/native_client/pnacl-subzero/).

Subzero for SwiftShader
-----------------------

SwiftShader contains a fork of the Subzero source code (at the time of writing they are in sync). It is an alternative JIT compiler back-end, with LLVM still being the default for CMake builds. To build SwiftShader with Subzero instead of LLVM, specify -DREACTOR_BACKEND=Subzero in your CMake command (or change LLVM to Subzero in the CMake GUI). For Chrome builds that use the BUILD.gn files, Subzero is the default as it produces significantly smaller binaries than with LLVM.

Subzero Development
-------------------

Development on Subzero itself requires setting up the NaCl environment on a Linux system to be able to run its unit tests:

* Install Chrome's [depot_tools](http://dev.chromium.org/developers/how-tos/install-depot-tools).
* Run `mkdir nacl && cd nacl && fetch nacl` ([ref](http://www.chromium.org/nativeclient/how-tos/how-to-use-git-svn-with-native-client)).
* Run `native_client/toolchain_build/toolchain_build_pnacl.py --verbose --sync --clobber --install toolchain/linux_x86/pnacl_newlib_raw` ([ref](https://sites.google.com/a/chromium.org/dev/nativeclient/pnacl/developing-pnacl#TOC-TL-DR-for-checking-out-PNaCl-sources-building-and-testing)).
* Run all unit tests with `make -f Makefile.standalone check` ([ref](https://chromium.googlesource.com/native_client/pnacl-subzero/+/master/docs/README.rst)).
