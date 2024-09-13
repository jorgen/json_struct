from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, CMake, cmake_layout
from conan.tools.env import VirtualBuildEnv, VirtualRunEnv
from conan.tools.scm import Git, Version
from conan.tools.files import copy

import os

class JsonStructLibrary(ConanFile):
    name = "json_struct"

    # Metadata
    license = "MIT"
    version = "1.0.0"
    author = "Jørgen Lind <jorgen.lind@gmail.com>"
    url = "https://github.com/jorgen/json_struct"
    description = "json_struct is a single header only C++ library for parsing JSON directly to C++ structs and vice versa"

    topics = ("serialization", "deserialization", "reflection", "json")

    settings = "os", "compiler", "build_type", "arch"
    pacgake_type = "header-library"
    implements = ["auto_header_only"]
    exports_sources = "include/*", "cmake/*" "CMakeLists.txt"

    options = {
        "opt_build_benchmarks": [True, False],
        "opt_build_examples": [True, False],
        "opt_build_tests": [True, False],
        "opt_disable_pch": [True, False],
        "opt_install": [True, False],
    }

    default_options = {
        "opt_build_benchmarks": False,
        "opt_build_examples": False,
        "opt_build_tests": False,
        "opt_disable_pch": False,
        "opt_install": True,
    }

    def generate(self):
        toolchain = CMakeToolchain(self)

        toolchain.variables["JSON_STRUCT_OPT_BUILD_BENCHMARKS"] = self.options.opt_build_benchmarks.value
        toolchain.variables["JSON_STRUCT_OPT_BUILD_EXAMPLES"] = self.options.opt_build_examples.value
        toolchain.variables["JSON_STRUCT_OPT_BUILD_TESTS"] = self.options.opt_build_tests.value
        toolchain.variables["JSON_STRUCT_OPT_DISABLE_PCH"] = self.options.opt_disable_pch.value
        toolchain.variables["JSON_STRUCT_OPT_INSTALL"] = self.options.opt_install

        toolchain.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        # Invoke cmake --install
        cmake = CMake(self)
        cmake.install()

    def layout(self):
        cmake_layout(self)

    def package_id(self):
        self.info.clear()
