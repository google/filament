# flex and bison binaries

This folder contains the flex and bison binaries. We use these binaries to
generate the ANGLE translator's lexer and parser.

Use the script [`update_flex_bison_binaries.py`](update_flex_bison_binaries.py)
to update the versions of these binaries in cloud storage. It must be run on
Linux or Windows. It will update the SHAs for your platform. After running the
script run `git commit` and then `git cl upload` to code review using the normal
review process. You will also want to run
[`scripts/run_code_generation.py`](../../scripts/run_code_generation.py) to
update the generated files.

Please update Windows, Linux and Mac binaries at the same time. Start with
Windows, then merge your work into a single CL that updates all binaries
simultaneously.

If you get authentication errors while uploading, try running:
```
$ download_from_google_storage --config
```

Contact syoussefi for any help with updating the binaries.

## Updating flex and bison binaries

This is expected to be a rare operation, and is currently done based on the
following instructions.  Note: get the binaries first on windows, as only a
single option is available, then build the binaries for the same version on
Linux/Mac.

### On Windows

Install MSys2 (x86_64) from http://www.msys2.org/ on Windows.  `flex` should
already be installed.  Install bison:

```
$ pacman -S bison
```

Note the versions of flex and bison, so you can build the exact same versions on Linux.
For example:

```
$ flex --version
flex 2.6.4

$ bison --version
bison (GNU Bison) 3.8.2
```

The only dependencies outside /Windows/ from MSys2 should be the following:

```
msys-intl-8.dll
msys-iconv-2.dll
msys-2.0.dll
```

This can be verified with:

```
$ ldd /usr/bin/flex
$ ldd /usr/bin/bison
```

Additionally, we need the binary for m4 at `/usr/bin/m4`.

Copy all these 5 files to this directory:

```
$ cd angle/
$ cp /usr/bin/flex.exe \
     /usr/bin/bison.exe \
     /usr/bin/m4.exe \
     /usr/bin/msys-intl-8.dll \
     /usr/bin/msys-iconv-2.dll \
     /usr/bin/msys-2.0.dll \
     tools/flex-bison/windows/
```

Upload the binaries:

```
$ cd angle/
$ py tools/flex-bison/update_flex_bison_binaries.py
```

### On Linux

```
# Get the source of flex
$ git clone https://github.com/westes/flex.git
$ cd flex/
# Checkout the same version as msys2 on windows
$ git checkout v2.6.4
# Build
$ autoreconf -i
$ mkdir build && cd build
$ ../configure CFLAGS="-O2 -D_GNU_SOURCE"
$ make -j
```

```
# Get the source of bison
$ curl http://ftp.gnu.org/gnu/bison/bison-3.8.2.tar.xz | tar -xJ
$ cd bison-3.8.2
# Build
$ mkdir build && cd build
$ ../configure CFLAGS="-O2"
$ make -j
```

Note: Bison's [home page][Bison] lists ftp server and other mirrors.  If the
above link is broken, replace with a mirror.

Copy the 2 executables to this directory:

```
$ cd angle/
$ cp /path/to/flex/build/src/flex \
     /path/to/bison/build/src/bison \
     tools/flex-bison/linux/
```

Upload the binaries:

```
$ cd angle/
$ ./tools/flex-bison/update_flex_bison_binaries.py
```

### On Mac

Use homebrew to install flex and bison:
```
$ brew install flex bison
```

Find the install locations using `brew info`:
```
$ brew info bison
...
/opt/homebrew/Cellar/bison/3.8.2 (100 files, 3.7MB)
...
```

Copy the 2 executables to this directory:

```
$ cd angle/
$ cp -r \
     /path/to/bison/install/bin \
     /path/to/flex/install/bin \
     tools/flex-bison/mac/
```

Upload the binaries:

```
$ cd angle/
$ ./tools/flex-bison/update_flex_bison_binaries.py
```

[Bison]: https://www.gnu.org/software/bison/
