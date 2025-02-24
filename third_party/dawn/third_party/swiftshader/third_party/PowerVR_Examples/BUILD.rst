==================
Build Instructions
==================
The following provides instructions for building the PowerVR SDK for Android, Windows, Linux, macOS or iOS platforms.

We provide a top level ``CMakeLists.txt`` and an ``android-build`` folder at the root of the SDK, which can be used to build the SDK for Windows, Linux, macOS and iOS or Android respectively. 
Alternatively the CMakeLists.txt and android-build folder can be used to incorporate parts of the SDK for Windows, Linux, macOS and iOS or Android respectively.

Each example in the SDK can also be built separately and contains:

* A CMakeLists.txt
* An ``android-build`` folder which contains a set of Android build scripts.

.. contents:: Index
   :depth: 3

Downloading the repository
--------------------------
Github
~~~~~~
To create a local git repository use:
``git clone https://github.com/powervr-graphics/Native_SDK.git``

Installer
~~~~~~~~~
Download the platform specific PowerVR SDK Installer from ``https://www.imgtec.com/developers/powervr-sdk-tools/installers/``

Repository Dependencies
-----------------------
Other than the platform specific build tools specified below the SDK satisfies all of its own required dependencies internally either by distributing them as part of the SDK or via the use of cmake external projects (externalproject_add).

Platform setup
--------------
Android
~~~~~~~
* Download and install the Android SDK or command-line only Android build tools
* Through the Android SDK Manager, either via Android Studio or command-line SDK manager, install the following packages:
     * Android NDK bundle
     * The Android SDK Platform package for API level 26 (used as our targetSdkVersion)
	 * The Android SDK Build-Tools version 29 (used as our compileSdkVersion)
     * CMake
     * LLDB [optional] - only required for on-device debugging
     * If you plan on using gradleW from the command-line make sure that %JavaHome% points to a valid Java JDK directory 

Windows, Linux and macOS
~~~~~~~~~~~~~~~~~~~~~~~~
* Download and install `CMake <https://cmake.org/download>`__
	* Version 3.3 or above is required.
	 
Windows
.......
* Download and install Visual Studio
	* Versions known to be supported 2015, 2017, 2019
	* Community versions should suffice with more capable versions also being supported. 
* The SDK has been built and tested on Windows 10 using Visual Studio versions 2015, 2017 and 2019 as well as MinGW.
	* Other versions of Windows may also work.
	* Other Windows-based build systems may also work.
	
Linux
.....
* Ensure system installed packages including build tools, window systems etc. are installed appropriately
	* This may include X11 packages, Wayland packages, libc++, libdl, and other libraries depending on the build configuration.
* The SDK has been built and tested on Ubuntu 16.04 and Ubuntu 18.04 LTS versions.
	* Other Linux distributions may also work else adaptions to the SDK should be straightforward.
	
macOS
.....
* Download a version of Apple's iOS SDK from `http://developer.apple.com/ios/ <http://developer.apple.com/ios/>`__. It is necessary to become a member of Apple's developer program in order to access this page. Details of how to join can be found at http://developer.apple.com.
* Install the Apple SDK on the Mac as specified by Apple's instructions. This will include Xcode and any other development tools required.
* To build for an iOS device, a valid Apple developer certificate is required in the machine's keychain. The ``Properties | Identifier`` property may need to be changed from ``Project | Edit Active Target...`` to match what was set up through Apple's Program Portal.
* If you do not have a developer certificate from Apple, then it is still possible to build and launch applications in the iOS Simulator. Choose this configuration from the dropdown menu at the top left and then choose Build and Run from the dropdown menu.

***Note:** The Scheme being built under may need to be set up for the SDK's projects to run.
	  
Build Options
-------------
Several options can be used to customise the build of the SDK or to control which modules/examples are built. Some of these options are binary enable/disable whilst others require the use of strings as inputs. 
The following table outlines the various options available:  

The following build options can be passed to CMake via the command line using the ``-D[PARAM_NAME]=[PARAM_VALUE]`` syntax alternatively these parameters can be configured using the CMake GUI.

.. _table1:
.. table:: CMake Build Options

    ======================================================= ============== ============== ==============
     **Option**                                              **Platform**   **Default**    **Comment**
    ======================================================= ============== ============== ==============
     ``CMAKE_BUILD_TYPE``                                    All            ``Release``    The build variant. Supported values: [Debug, Release, MinSizeRel, RelWithDebInfo]
    ------------------------------------------------------- -------------- -------------- --------------
     ``PVR_BUILD_EXAMPLES``                                  All            ``On``         Determines whether the PowerVR SDK examples are built
    ------------------------------------------------------- -------------- -------------- --------------
     ``PVR_BUILD_FRAMEWORK``                                 All            ``On``         Determines whether the PowerVR SDK framework modules are built
    ------------------------------------------------------- -------------- -------------- --------------
     ``PVR_BUILD_OPENGLES_EXAMPLES``                         All            N/A            Pass this parameter if both Vulkan and OpenGL examples are downloaded but, for whatever reason, only a solution for the OpenGL ES ones is required
    ------------------------------------------------------- -------------- -------------- --------------
     ``PVR_BUILD_VULKAN_EXAMPLES``                           All            N/A            Pass this parameter if both Vulkan and OpenGL examples are downloaded but, for whatever reason, only a solution for the Vulkan ones is required
    ------------------------------------------------------- -------------- -------------- --------------
     ``WS`` (Deprecated - Please prefer PVR_WINDOW_SYSTEM)   Linux/QNX      N/A            Can be used to control the windowing system used. Supported values: [NullWS, X11, Wayland, Screen]. Usually, desktop Linux systems will be running an X11/XCB or using a Wayland server. Development platforms often use a NullWS system which is where the GPU renders to the screen directly without using a windowing system. Screen is commonly used on QNX.
    ------------------------------------------------------- -------------- -------------- --------------
     ``PVR_WINDOW_SYSTEM``                                   Linux/QNX      N/A            Can be used to control the windowing system used. Supported values: [NullWS, X11, Wayland, Screen]. Usually, desktop Linux systems will be running an X11/XCB or using a Wayland server. Development platforms often use a NullWS system which is where the GPU renders to the screen directly without using a windowing system. Screen is commonly used on QNX.
    ------------------------------------------------------- -------------- -------------- --------------
     ``PVR_GLSLANG_VALIDATOR_INSTALL_DIR``                   All            N/A            This parameter can be used to provide a directory containing a glslangValidator binary which will be used instead of rebuilding it from source
    ------------------------------------------------------- -------------- -------------- --------------
     ``PVR_PREBUILT_DEPENDENCIES``                           All            N/A            This parameter can be used to avoid building the dependencies for the current module or example on which this option was used instead the dependency will be found using cmake's find_package logic. This parameter should not generally be used and is primarily used for optimising android builds.
    ======================================================= ============== ============== ==============

The following build options can be passed via gradle using the ``-P[PARAM_NAME]=[PARAM_VALUE]`` syntax.

.. _table2:
.. table:: Gradle Build Options

     ======================= ====================================== ==============
      **Option**              **Default**                            **Comment**
     ======================= ====================================== ==============
      ``KEYSTORE``            N/A                                    This parameter can be used to provide a path to an android keystore file used for signing a release built Android apk
     ----------------------- -------------------------------------- --------------
      ``KEYSTORE_PASSWORD``   N/A                                    This parameter can be used to provide a password for a given android keystore file used for signing a release built Android apk
     ----------------------- -------------------------------------- --------------
      ``KEY_ALIAS``           N/A                                    This parameter can be used to provide an alias for a given android keystore file used for signing a release built Android apk
     ----------------------- -------------------------------------- --------------
      ``KEY_PASSWORD``        N/A                                    This parameter can be used to provide the password for a key in the given android keystore file used for signing a release built Android apk
     ----------------------- -------------------------------------- --------------
      ``NOSIGN``              N/A                                    This parameter can be used to disable signing of release built Android apks
     ----------------------- -------------------------------------- --------------
      ``ANDROID_ABIS``        ``x86,x86_64,armeabi-v7a,arm64-v8a``   This parameter can be used to specify the target architectures which will be built
     ======================= ====================================== ==============

See the CMake and gradle documentation for more information on advanced usage.
	
Cross Compiliation
------------------
CMake uses toolchain files for cross-compiling. These are usually not necessary when targeting the machine that is being built on, also known as native or host compilation.
For cross-compiling, The SDK includes a number of CMake toolchain files in ``[path-to-sdk]/cmake/toolchains``. Alternatively these toolchain files can be used as a reference for making other toolchain files. 
Toolchains are passed directly to the CMake command-line: ``cmake ../.. -DCMAKE_TOOLCHAIN_FILE=[path-to-sdk]/cmake/toolchains/Linux-gcc-armv8.cmake`` 

The SDK provides toolchain files for the following architectures/platforms:
    * ios
	* Linux
		* armv7
		* armv7hf
		* armv8
		* mips\_32
		* mips\_64
		* x86\_32
		* x86\_64
	* QNX
		* aarch64le
		* armle-v7
		* x86\_32
		* x86\_64

Building on Windows, Linux and macOS
------------------------------------
CMake build Quickstart
~~~~~~~~~~~~~~~~~~~~~~
The following can be used to build the SDK using system and platform specific defaults on a Unix-based system:

.. code-block:: bash

	git clone https://github.com/powervr-graphics/Native_SDK.git
	cd Native_SDK
	mkdir build
	cd build
	cmake ..
	cmake --build .

**Note:** The ``build`` folder can be replaced a path to ``any-folder``
**Note:** The ``mkdir`` command can be replaced with an ``md`` on Windows

Alternatively the cmake configuration step can make use of one or more of the build options outlined above.

CMake Configuration Instructions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Create a directory to use for the files CMake will generate, and navigate to this directory. 
* Execute CMake, pointing it to the directory where the ``CMakeLists.txt`` is located.

For example: from ``[path-to-sdk]/cmake-build/``, or from ``[path-to-sdk]/examples/[example_api]/[example_name]/cmake-build/`` folder:

  ``cmake ..`` (optionally specifying the CMake Generator i.e. ``-G`` Unix Makefiles, Visual Studio, Xcode, Eclipse, Ninja etc. and architecture)

Windows Visual Studio
.....................
Microsoft Visual Studio is the default generator on Windows. CMake cannot generate multi-architecture projects (ones that support both 32-bit and 64-bit) as is conventional for those familiar with MSVC, so only one can be selected. It is recommended to use 64-bit if it is available, but both are fully supported. 

The default CMake architecture is 32-bit. It can be set to 64-bit by passing the ``-A[x64]`` parameter.

* ``cmake [path-to-directory-containing-CMakeLists.txt]`` - generates a solution for the installed version of Visual Studio, 32-bit
* ``cmake [path-to-directory-containing-CMakeLists.txt] -Ax64`` - generates a solution for the installed version of Visual Studio, 64-bit
* ``cmake [path-to-directory-containing-CMakeLists.txt] -G "Visual Studio 15" -Ax64`` - generates Visual Studio 2017 solution, 64-bit
* ... and so on

Xcode (when targetting macOS)
.............................
In order to generate Xcode projects, the Xcode generator must be explicitly passed:

``cmake [path-to-directory-containing-CMakeLists.txt] -G Xcode``

The generated project files can be opened with Xcode as normal, or built from command-line with ``xcodebuild`` or ``cmake --build .``

Xcode (when targeting iOS)
..........................
The instructions for iOS are the same as for macOS except a CMake toolchain file needs to be passed, as iOS is a cross-compiled target, and a code sign identity needs to be specified. The PowerVR SDK provides an iOS toolchain file: ``[path-to-sdk]/cmake/toolchains/Darwin-gcc-ios.cmake``. 
To appropriately compile the SDK the following options must be set in the toolchain file ``ENABLE_ARC=0`` and ``IOS_PLATFORM=OS64`` which are used for disabling Automatic Reference Counting (ARC) and for targeting only 64bit platforms including arm64 and arm64e iPhoneOS respectively. 
To specify a code sign identity the following options must to be set ``CODE_SIGN_IDENTITY=[IDENTITY]`` and ``DEVELOPMENT_TEAM_ID=[TEAM_ID]``. These options can also be set at a later time from the Xcode IDE.

Generate the Xcode projects with:

``cmake [path-to-directory-containing-CMakeLists.txt] -G Xcode -DCMAKE_TOOLCHAIN_FILE=[path-to-sdk]/cmake/toolchains/Darwin-gcc-ios.cmake -DENABLE_ARC=0 -DIOS_PLATFORM=OS64``

Building Generated Projects
~~~~~~~~~~~~~~~~~~~~~~~~~~~
The projects can be built as usual based on the types of projects selected, such as through Visual Studio or calling ``make`` for the makefiles or alternatively can be built using ``cmake -- build .``

Binaries are output to the ``bin`` subfolder of the CMake binary folder or ``android-build`` folder.

Unix Makefiles
..............
Unix makefiles are the default way to build on Linux, but also work anywhere a ``make`` program exists.
Building the project is performed by calling ``make [-j8 , other options]``

**Note:** The use of multithreaded builds using ``-j[some number]`` is recommended when building with makefiles as it can speed up the build *considerably*.

Android Build Instructions
--------------------------
Android uses its own build system, which uses CMake internally. Instead of calling CMake directly, Gradle is used which makes use of CMake as appropriate internally.

The easiest way to build, run, and debug Android applications is to download and use Android Studio from Google. This is highly recommended, if nothing else for the easy on-device debugging that it offers.
Alternatively building from the command-line is also very easy. The ``gradle wrapper`` is used to avoid downloading and installing ``gradle``. The wrapper is a small script located in the corresponding ``build-android`` folder. The wrapper will automatically download (if not present) the required Gradle version and run it.
**Note:** Using the Gradle wrapper is optional, Gradle can still be downloaded, installed and used manually.

* To build from Android Studio, use the ``Import project`` dialog, and select the desired ``build-android`` folder for the SDK, a particular example or a framework module.
	* The required Gradle build scripts will be found in the ``[path-to-sdk]/build-android`` folder, in each example's corresponding ``build-android`` folder or in the framework module's corresponding ``build-android`` folder. 
	* Android Studio will prompt for any missing packages when attempting to build.
* To build from command-line, navigate to the ``build-android`` folder and run ``gradlew assemble[Debug/Release]``
	* Create a ``local.properties`` file, and add the line ``sdk-dir=[path-to-the-ANDROID-sdk]``, or add an environment variable ``ANDROID_HOME=[path-to-the-ANDROID-sdk]``.

Android Quickstart
~~~~~~~~~~~~~~~~~~
Using the Gradle wrapper:

* Run ``gradlew assemble[Debug/Release] [parameters]`` from the ``build-android`` folder

Using Gradle directly:

* Download, install, and add Gradle to the path
* Run ``gradle assemble[Debug/Release] [parameters]`` from the ``build-android`` folder

Gradle properties
~~~~~~~~~~~~~~~~~
There are a few different properties that can/need to be configured. These can be set up in different places:

* A ``gradle.properties`` file in each example or framework module configures properties for that project.
* A global ``gradle.properties`` file in the ``GRADLE_USER_HOME`` directory. This is not provided, but it is very convenient to globally override all the SDK options. For example - key signing, or for changing the target Android ABI for the whole SDK.
* Individual properties can be passed as command-line parameters, by passing ``-P[PARAM_NAME]=[PARAM_VALUE]`` to the command-line.

Android ABIs
............
By default, every example's ``gradle.properties`` file has an ``ANDROID_ABIS=x86,x86_64,armeabi-v7a,arm64-v8a`` entry. This creates an apk that targets those architectures.

During development it is often preferable to build only for a single platform's architecture to decrease build times. To change the architectures which are built, there are several options:

* Change the properties in each required project 's gradle.properties file
* Add a corresponding line to the global ``gradle.properties`` file. This overrides per-project properties.
* Build with, for example, ``gradlew assembleDebug -PANDROID_ABIS=armeabi-v7a``. This overrides both ``gradle.properties`` files.

APK Signing
...........
The provided Gradle scripts have provision for signing the release apks. This is achieved by setting properties in your apks. We recommend that if you set up your own keystore, add your usernames and key aliases to a global ``gradle.properties``, and pass the passwords through the command-line. 

The following properties must be set either per project in per-project ``gradle.properties``, or globally in system-wide ``gradle.properties`` or through the command-line with ``-PNOSIGN``:

* ``KEYSTORE=[Path-to-keystore-file]``
* ``KEYSTORE_PASSWORD=[Password-to-keystore]``
* ``KEY_ALIAS=[Alias-to-signing-key]``
* ``KEY_PASSWORD=[Password-to-signing]``

If the release apks do not need to be signed, pass the parameter ``NOSIGN`` with any value to disable signing:

* ``NOSIGN=[1]``