# How to build ANGLE in Chromium for dev

## Introduction

On Windows, Linux, and Mac ANGLE now builds most core components cross platform, including the shader validator and translator as well as the graphics API translator. These parts can be built and tested inside a Chromium checkout.

ANGLE also includes some sample applications and a few other targets that don't build on Chromium. These steps describe how to build such targets within a Chromium checkout.

Prerequisite Steps:

  * Checkout and build [Chromium](http://dev.chromium.org/Home).
  * To setup run these commands (note similarity to [DevSetup](DevSetup.md)):

## Standalone ANGLE inside Chromium

  * To sync all standalone dependencies run:

```bash
cd src/third_party/angle
python3 scripts/bootstrap.py
gclient sync
```

  * To generate ANGLE standalone build files run:

```bash
cd src/third_party/angle
gn gen out/Debug
```

  * To build:

```bash
cd src/third_party/angle
ninja -j 10 -k1 -C out/Debug
```

  * For example, `ninja -j 10 -k1 -C out/Debug angle_gles2_deqp_tests`
  * To run a sample application: `./out/Debug/hello_triangle`
  * To go back to the Chromium-managed version, remove `third_party/angle/.gclient`.

## Working with ANGLE in Chromium

You will also want to work with a local version of ANGLE instead of the version that is pulled in by Chromium's [DEPS](https://chromium.googlesource.com/chromium/src/+/main/DEPS) file. To do this do the following:

  * cd to `chromium/`. One directory above `chromium/src`. Add this to `chromium/.gclient`:

```python
solutions = [
  {
    # ...
    u'custom_deps':
    {
      "src/third_party/angle": None,
    },
  },
]
```

You will have full control over your ANGLE workspace and are responsible for running all git commands (pull, rebase, etc.) for managing your branches.

If you decide you need to go back to the DEPS version of ANGLE:

  * Comment out or remove the `src/third_party/angle` line in your `custom_deps` in `chomium/.gclient`.
  * Se the ANGLE workspace to the version specified in Chromium's DEPS. Ensure there are no modified or new files.
  * `gclient sync` your Chromium workspace.
