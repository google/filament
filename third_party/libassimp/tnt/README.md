To update assimp use the following steps:

- Download or checkout Assimp's source code [assimp.org](http://www.assimp.org) or from
  [GitHub](https://github.com/assimp/assimp)
- In the Assimp directory, create an `out/` directory and from there run
  `cmake .. -DCMAKE_BUILD_TYPE=Release`, then `make -j`
- From `out/` copy `revision.h` to the root of the assimp source tree
- From `out/` copy `include/assimp/config.h` to `include/assimp/config.h` in the root of the assimp source tree
- Open `tnt/CMakeLists.txt` and update the lists of public headers, private headers and source
  files. All files present on disk can be listed but to speed up build times you should remove
  source files related to importers/exporters we do not use
- Make sure to update the list of definitions that start with `-DASSIMP_BUILD_NO_` to skip all
  the importers we do not want to use
- Make sure the definition `-DASSIMP_BUILD_NO_EXPORTER` is still there to remove all exporters

In addition we recommend to remove the following directories:
- `samples/`
- `test/`
- `tools/`

We do not use these directories and they take up unnecessary space on disk.

Other Changes
-------------

(1)

Negate the bitangent in CalcTangentsProcess.cpp (see Filament commit 84eb954).
