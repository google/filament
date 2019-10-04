## Updating

To update to the SPIRV-Tools that's currently on GitHub master, do the following from the
`third_party` directory.

### Update SPIRV-Tools

```
curl -L https://github.com/KhronosGroup/SPIRV-Tools/archive/master.zip > master.zip
unzip master.zip
rsync -r SPIRV-Tools-master/ spirv-tools/ --delete
git checkout spirv-tools/tnt/
rm -rf SPIRV-Tools-master master.zip
```

### Make Filament-specific changes

SPIRV-Tools requires Python 3 for running unit tests. We don't need to build or run tests, so make
the following chnange to the `spirv-tools/CMakeLists.txt` file:

```
# Tests require Python3
# ~~~ Begin Filament change, tests not needed
# find_host_package(PythonInterp 3 REQUIRED)
# ~~~ End Filament change
```

To avoid a CMake warning about rpaths, also this additional change to `spirv-tools/source/CMakeLists.txt`:

```
# ~~~ Begin Filament change, shared libraries not needed
# add_library(${SPIRV_TOOLS}-shared SHARED ${SPIRV_SOURCES})
# spvtools_default_compile_options(${SPIRV_TOOLS}-shared)
# target_include_directories(${SPIRV_TOOLS}-shared
#   PUBLIC
#     $<BUILD_INTERFACE:${spirv-tools_SOURCE_DIR}/include>
#     $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/include>
#   PRIVATE ${spirv-tools_BINARY_DIR}
#   PRIVATE ${SPIRV_HEADER_INCLUDE_DIR}
#   )
# set_target_properties(${SPIRV_TOOLS}-shared PROPERTIES CXX_VISIBILITY_PRESET hidden)
# set_property(TARGET ${SPIRV_TOOLS}-shared PROPERTY FOLDER "SPIRV-Tools libraries")
# spvtools_check_symbol_exports(${SPIRV_TOOLS}-shared)
# target_compile_definitions(${SPIRV_TOOLS}-shared
#   PRIVATE SPIRV_TOOLS_IMPLEMENTATION
#   PUBLIC SPIRV_TOOLS_SHAREDLIB
# )
# add_dependencies( ${SPIRV_TOOLS}-shared core_tables enum_string_mapping extinst_tables )
# ~~~ End Filament change, install not needed
```

and this change to `spirv-tools/CMakeLists.txt`:

```
# ~~~ Begin Filament change, install not needed
option(SKIP_SPIRV_TOOLS_INSTALL "Skip installation" ON)
# ~~~ End Filament change
```

### Update SPIRV-Headers

```
curl -L https://github.com/KhronosGroup/SPIRV-Headers/archive/master.zip > master.zip
unzip master.zip
rsync -r SPIRV-Headers-master/ spirv-tools/external/spirv-headers --delete
rm -rf SPIRV-Headers-master master.zip
```

### Commit
```
git add spirv-tools
git commit -m "Update SPIRV-Tools"
```

Please be sure to test Filament before uploading your PR.
