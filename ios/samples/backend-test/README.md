# iOS `backend-test` App

The `backend-test` app is capable of running unit tests defined in filament/backend/test on an iOS
device. These tests are designed to exercise `libbackend`, Filament's backend library. Tests are
written in files like `test_MissingRequiredAttributes.cpp`. Tests can call driver commands and
render one or more "frames" to invoke functionality. For example, the MissingRequireAttributes test
checks that backends can handle shaders with vertex attributes that aren't set in a renderable.

In order to run the test cases on iOS, a library called `libbackendtest` must be built for iOS.
This library can be built by setting the `INSTALL_BACKEND_TEST` CMake flag to `ON`. For example, to
turn the flag on for iOS debug builds,

```
cd out/cmake-ios-debug-arm64/
cmake . -DINSTALL_BACKEND_TEST=ON
cd ../..
./build.sh -i -p ios debug
```

You can choose to run the test cases either under the OpenGL or Metal backend by choosing the
appropriate scheme inside the Xcode project. The --gtest_filter argument can be used to control
which test case is run (at the moment, running multiple tests causes some issues on some backends).
To set arguments in Xcode, navigate to Product -> Scheme -> Edit Scheme... -> Arguments.
