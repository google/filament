# assimp for iOS
(deployment target 6.0+, 32/64bit)

Builds assimp libraries for several iOS CPU architectures at once, and outputs a fat binary from the result.

Run the **build.sh** script from the ```./port/iOS/``` directory. See **./build.sh --help** for information about command line options. 

```bash
shadeds-Mac:iOS arul$ ./build.sh --help
[!] ./build.sh - assimp iOS build script
 - don't build fat library (--no-fat)
 - supported architectures(--archs): armv7, armv7s, arm64, i386, x86_64
 - supported C++ STD libs.(--stdlib): libc++, libstdc++
```
Example:
```bash
cd ./port/iOS/
./build.sh --stdlib=libc++ --archs="armv7 arm64 i386"
```
Supported architectures/devices:

### Simulator
- i386
- x86_64
 
### Device
- ~~ARMv6 (dropped after iOS 6.0)~~
- ARMv7
- ARMv7-s
- ARM64

### Building with older iOS SDK versions
The script should work out of the box for the iOS 8.x SDKs and probably newer releases as well.
If you are using SDK version 7.x or older, you need to specify the exact SDK version inside **build.sh**, for example:
```
IOS_SDK_VERSION=7.1
```
### Optimization
By default, no compiler optimizations are specified inside the build script. For an optimized build, add the corresponding flags to the CFLAGS definition inside **build.sh**.
