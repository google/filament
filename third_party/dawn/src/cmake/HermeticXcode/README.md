This is a cobbled-together CMake toolchain file for using "hermetic Xcode",
rather than the system-installed one. It is not used by default. It is meant to
be used on automated builder bots to avoid depending on system dependencies, but
can also be used locally.

Use this by passing `-DCMAKE_TOOLCHAIN_FILE=path/to/HermeticXcode.cmake` to
`cmake`.

Notes:

- Not all of the variables set in the script are probably actually used
  (and thus could break later on if they become used), but are included in an
  attempt at completeness.
- `ranlib` is a link to `libtool` because those are the same binary with
  different names. (It needs to be a symlink because it chooses its behavior
  based on its filename.)
- It doesn't currently try to set up other things as hermetic, in particular
  Python.
