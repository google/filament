# Wasm Prototype Experiment Notes

Here's the command I use to test:

```
LD_LIBRARY_PATH=~/nacl/v8/out/native/lib.target make -j48 \
    -f Makefile.standalone WASM=1 && \
LD_LIBRARY_PATH=~/nacl/v8/out/native/lib.target ./pnacl-sz -O2 -filetype=asm \
    -target=arm32 ./torture-s2wasm-sexpr-wasm/20000112-1.c.s.wast.wasm
```

You'll probably need to adjust your `LD_LIBRARY_PATH` to point to where your v8
libraries are.

You'll need to build v8 as a shared library. Build it like this:

```
make -j48 native component=shared_library
```

`wasm-run-torture-tests.py` can be used to run all the tests, or some
subset. Running a subset will enable verbose output. You can download the
torture tests from the [WebAssembly waterfall](https://wasm-stat.us/console) or
by running `./fetch-torture-tests.sh`.


To compile and run an executable, do the following from the subzero directory.

```
./pnacl-sz -filetype=obj -o foo.o foo.wasm
clang -m32 foo.o src/wasm-runtime.c
./a.out
```

Other useful commands:

Compile a C file to a .wasm
```
./wasm-install/bin/emscripten/emcc hello-wasm.c -s BINARYEN=1
./wasm-install/bin/sexpr-wasm a.out.wast -o a.out.wasm
```
