// swift-tools-version:5.5
// The swift-tools-version declares the minimum version of Swift required to build this package.

// Copyright 2016-2021 The Khronos Group Inc.
// SPDX-License-Identifier: Apache-2.0

import PackageDescription

let package = Package(
    name: "SPIRV-Cross",
    products: [
        // Products define the executables and libraries a package produces, and make them visible to other packages.
        .library(
            name: "SPIRV-Cross",
            targets: ["SPIRV-Cross"]),
    ],
    dependencies: [
        // Dependencies declare other packages that this package depends on.
        // .package(url: /* package url */, from: "1.0.0"),
    ],
    targets: [
        // Targets are the basic building blocks of a package. A target can define a module or a test suite.
        // Targets can depend on other targets in this package, and on products in packages this package depends on.
        .target(
            name: "SPIRV-Cross",
            dependencies: [],
            path: ".",
            exclude: ["CMakeLists.txt",
                      "CODE_OF_CONDUCT.adoc",
                      "LICENSE",
                      "LICENSES",
                      "Makefile",
                      "README.md",
                      "appveyor.yml",
                      "build_glslang_spirv_tools.sh",
                      "checkout_glslang_spirv_tools.sh",
                      "cmake",
                      "format_all.sh",
                      "gn",
                      "main.cpp",
                      "pkg-config",
                      "reference",
                      "samples",
                      "shaders",
                      "shaders-hlsl",
                      "shaders-hlsl-no-opt",
                      "shaders-msl",
                      "shaders-msl-no-opt",
                      "shaders-no-opt",
                      "shaders-other",
                      "shaders-reflection",
                      "shaders-ue4",
                      "shaders-ue4-no-opt",
                      "test_shaders.py",
                      "test_shaders.sh",
                      "tests-other",
                      "update_test_shaders.sh"],
            sources: ["spirv_cfg.cpp",
                      "spirv_cpp.cpp",
                      "spirv_cross.cpp",
                      "spirv_cross_c.cpp",
                      "spirv_cross_parsed_ir.cpp",
                      "spirv_cross_util.cpp",
                      "spirv_glsl.cpp",
                      "spirv_hlsl.cpp",
                      "spirv_msl.cpp",
                      "spirv_parser.cpp",
                      "spirv_reflect.cpp"],
            publicHeadersPath: "."),
    ],
    cxxLanguageStandard: .cxx14
)
