# Shader substitution

ANGLE provides two mechanisms for observing, modifying, and substituting
the application's shaders. This ability to interpose makes it easier to
diagnose bugs and to prototype new transforms in the shader translator.

## Environment variables controlling reading/writing shaders to disk

For both the source and translated shaders discussed below, the environment
variable:

```
ANGLE_SHADER_DUMP_PATH
```

and the Android property:

```
debug.angle.shader_dump_path
```

specify the directory in which shader sources and translated shaders will
be written to, and, in the case of shader substitution, read from. For
example, on non-Android platforms:

```
mkdir -p /path/to/angle_shaders
export ANGLE_SHADER_DUMP_PATH=/path/to/angle_shaders
```

will write all data to the `angle_shaders` directory.

On Android, it's necessary to set the `debug.angle.shader_dump_path` property
and set up the SD card correctly. (Help expanding this documentation is
appreciated!)

## ESSL shader dumping and substitution

The ANGLE feature `dumpShaderSource`, when enabled, writes all incoming
ESSL shader sources to disk, in the shader dump directory specified
above. File names are computed by hashing the shader sources. Shaders will
only be written to disk if they were not loaded from disk via substitution,
below.

The ANGLE feature `enableShaderSubstitution`, when enabled, looks for a
file in the shader dump directory where the filename is the hash of the
application's shader source, and substitutes its contents for the
application's shader. This allows you to dump and edit these files at your
leisure, and rerun the application to pick up the new versions of the
shaders.

In Chromium, pass the following command line arguments to enable these
features:

```
--enable-angle-features=dumpShaderSource
--enable-angle-features=enableShaderSubstitution
--enable-angle-features=dumpShaderSource,enableShaderSubstitution
```

You must also specify `--disable-gpu-sandbox` to allow ANGLE to access
these on-disk files for reading and writing. **Do not** browse the open web
with this command line argument specified!

## Translated shader dumping and substitution

The translated shaders produced by ANGLE's shader translator can be dumped
and substituted as well. This is especially useful when prototyping new
optimizations in the shader translator.

This mechanism is relatively recent and has not been thoroughly tested. It
will likely not work in the situation where ANGLE dynamically recompiles
shaders internally. It should work with all text-based shader translator
backends (ESSL, GLSL, HLSL, and Metal). See comments in
`src/libANGLE/Shader.cpp` describing the work needed to make this work with
the SPIR-V backend.

Translated shaders go into the same shader dump directory specified above
for shader sources. To enable these features:

```
--enable-angle-features=dumpTranslatedShaders,enableTranslatedShaderSubstitution --disable-gpu-sandbox
```

## Putting it all together (example: macOS)

```
mkdir -p $HOME/tmp/angle_shaders
export ANGLE_SHADER_DUMP_PATH=$HOME/tmp/angle_shaders

out/Release/Chromium.app/Contents/MacOS/Chromium --disable-gpu-sandbox --use-angle=metal --enable-angle-features=dumpShaderSource,enableShaderSubstitution,dumpTranslatedShaders,enableTranslatedShaderSubstitution
```

Run the application once to generate the shader dump. Edit source or
translated shaders as desired. Rerun with the same command line arguments
to pick up the new versions of the shaders.

Alternatively, and especially if the application doesn't work with all of
the shaders in the substitution directory, make a new directory and copy in
only those source or translated shaders you want to substitute, and run:

```
out/Release/Chromium.app/Contents/MacOS/Chromium --disable-gpu-sandbox --use-angle=metal --enable-angle-features=enableShaderSubstitution,enableTranslatedShaderSubstitution
```
