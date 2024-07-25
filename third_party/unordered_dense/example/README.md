A simple example that demonstrats how to make use of `unordered_dense` with cmake.

Use globally installed `unordered_dense`:
```sh
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=${HOME}/unordered_dense_install ..
make
```

Use locall installed `unordered_dense`, as in the main README.md:

```sh
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=${HOME}/unordered_dense_install ..
make
```
