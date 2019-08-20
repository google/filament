#!/usr/bin/env python
# -*- coding: utf-8 -*
from conans import ConanFile, tools, CMake


class civetwebConan(ConanFile):
    name = "civetweb"
    license = "MIT"
    url = "https://github.com/civetweb/civetweb"
    description = "Embedded C/C++ web server"
    author = "Bernhard Lehner <bel2125@gmail.com>"
    topics = ("conan", "civetweb", "web-server", "embedded")
    exports = ("LICENSE.md", "README.md")
    exports_sources = ("src/*", "cmake/*", "include/*", "CMakeLists.txt")
    generators = "cmake"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared"            : [True, False],
        "fPIC"              : [True, False],
        "enable_ssl"        : [True, False],
        "enable_websockets" : [True, False],
        "enable_ipv6"       : [True, False],
        "enable_cxx"        : [True, False]
    }
    default_options = {
        "shared"            : False,
        "fPIC"              : True,
        "enable_ssl"        : True,
        "enable_websockets" : True,
        "enable_ipv6"       : True,
        "enable_cxx"        : True
    }

    def config_options(self):
        if self.settings.os == 'Windows':
            del self.options.fPIC

    def configure(self):
        if not self.options.enable_cxx:
            del self.settings.compiler.libcxx

    def requirements(self):
        if self.options.enable_ssl:
            self.requires("OpenSSL/1.0.2q@conan/stable")

    def _configure_cmake(self):
        cmake = CMake(self)
        cmake.verbose = True
        cmake.definitions["CIVETWEB_ENABLE_SSL"] = self.options.enable_ssl
        cmake.definitions["CIVETWEB_ENABLE_WEBSOCKETS"] = self.options.enable_websockets
        cmake.definitions["CIVETWEB_ENABLE_IPV6"] = self.options.enable_ipv6
        cmake.definitions["CIVETWEB_ENABLE_CXX"] = self.options.enable_cxx
        cmake.definitions["CIVETWEB_BUILD_TESTING"] = False
        cmake.definitions["CIVETWEB_ENABLE_ASAN"] = False
        cmake.configure(build_dir="build_subfolder")
        return cmake

    def build(self):
        tools.replace_in_file(file_path="CMakeLists.txt",
                              search="project (civetweb)",
                              replace="""project (civetweb)
                                 include(conanbuildinfo.cmake)
                                 conan_basic_setup()""")
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        self.copy("LICENSE.md", dst="licenses")
        cmake = self._configure_cmake()
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)
        if self.settings.os == "Linux":
            self.cpp_info.libs.extend(["dl", "rt", "pthread"])
            if self.options.enable_cxx:
                self.cpp_info.libs.append("m")
        elif self.settings.os == "Macos":
            self.cpp_info.exelinkflags.append("-framework Cocoa")
            self.cpp_info.sharedlinkflags = self.cpp_info.exelinkflags
            self.cpp_info.defines.append("USE_COCOA")
        elif self.settings.os == "Windows":
            self.cpp_info.libs.append("Ws2_32")
        if self.options.enable_websockets:
            self.cpp_info.defines.append("USE_WEBSOCKET")
        if self.options.enable_ipv6:
            self.cpp_info.defines.append("USE_IPV6")
        if not self.options.enable_ssl:
            self.cpp_info.defines.append("NO_SSL")
