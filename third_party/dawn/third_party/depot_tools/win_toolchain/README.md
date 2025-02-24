Automatic updates of the Windows toolchain
==========================================

## On the consumer side, e.g. in Chromium src.git:

- `vs_toolchain.py update` is called early during `DEPS`. `Update()` asks
  depot\_tools to put in place a particular version of the toolchain (whatever
  src will currently build with). src provides an output .json file, where
  `Update()` saves relevant information about the toolchain, the paths, version
  numbers, etc.
- Later in `DEPS`, `build/gyp_chromium` uses
  `vs_toolchain:SetEnvironmentAndGetRuntimeDllDirs()`, which loads the .json
  file, and uses it to set a few `GYP_` variables and update the `PATH` to
  include CRT runtime directories (see below).
- Then, `gyp_chromium` runs gyp generation.
- Finally, it uses `vs_toolchain` again to copy runtime dlls to the output
  directories.

The reason the logic was split between `depot_tools` and `src` was because at
some point, the bots had insufficient hard drive space and if there were > 1
build directories (say, if a build machine handled the Release and Debug builds
for a given configuration) then the duplication of the toolchain in both trees
would cause the bot to run out of disk space.

## On the depot\_tools side:

`get_toolchain_if_necessary.py` takes an output .json file (per above) and an
input SHA1. It tries to confirm that the user is probably a Google employee (or
a bot) to encourage them to use the automatic toolchain rather than using a
system installed one. It then uses gsutil to download the zip corresponding to
the hash. This requires authentication with @google.com credentials, so it walks
the user through that process if necessary.

(Previously in the VS2010 and early VS2013 timeframe, we also supported building
with Express editions of VS. Along with `toolchain2013.py` this script dealt
with all the complexity of acquiring the Express ISO, SDK bits, patches, etc.
and applying them all in the correct sequence. However, Express no longer works,
and Community is not too hard to install properly, so we just let the user do
that. The primary benefit of having an automatically updated toolchain is that
it works for bots, allows changes to the toolchain to be tryjob'd, reduces
Infra/Labs work, and ensures that devs match bots.)

For the above convoluted reason `get_toolchain_if_necessary` uses
`toolchain2013.py` to extract the zip file, but the majority of the code in
there is no longer used and what remains should be inlined into
`get_toolchain_if_necessary` in the future.

When the zip file is extracted, the mtimes of all the files, and the sha1 of the
entire tree are saved to a local file. This allows future updates to compare
whether the bits of the toolchain currently on disk are different than expected
(the passed in SHA1), and if so, replace it with a toolchain with the correct
SHA1. This is probably a bit more complicated than necessary, and again dates
back to when the toolchain was assembled from many pieces. It could probably
just write a stamp file with the SHA1, or just a version number, and trust that
on future runs.

Finally, it copies the json file to the location that the caller requested (the
json file is generated during the unzip/acquire process in `toolchain2013.py`).

## Building a <sha1>.zip

Ignoring the `toolchain2013.py` steps to acquire a toolchain automatically from
bits for Express, the procedure is roughly:
- Get a clean Windows VM,
- Install Visual Studio 2013 with updates as you want it,
- Install Windows 8.1 SDK,
- Run `package_from_installed.py`,
- Upload the resulting zip file to the chrome-wintoolchain GS bucket.

That script first builds a zip file of the required pieces, including generating
a batch file corresponding to `SetEnv.cmd` or `vcvarsall.bat`. It then extracts
that zip to a temporary location and calculates the SHA1 in the same way that
the `depot_tools` update procedure would do, so that it knows what to rename the
zip file to.
