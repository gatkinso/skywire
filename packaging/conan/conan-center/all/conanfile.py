import os

from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.files import copy, get, rm, rmdir

required_conan_version = ">=2.1"


class SkywireConan(ConanFile):
    name = "skywire"
    description = ("wiregen, a code generator for fixed-layout binary messages (C, C++, C#, Go, "
                   "Java, JavaScript, Python, Rust and Swift, with JSON, XML, BSON and protobuf), "
                   "and the header-only C and C++ runtimes the generated code uses")
    license = "Apache-2.0"
    url = "https://github.com/conan-io/conan-center-index"
    homepage = "https://github.com/gatkinso/skywire"
    topics = ("code-generator", "serialization", "binary-protocol", "icd", "bitfields",
              "json", "xml", "bson", "protobuf")
    # A tool (wiregen) and header-only runtimes for the code it generates; no
    # library. wiregen links protobuf statically, so consumers never see it.
    package_type = "unknown"
    settings = "os", "arch", "compiler", "build_type"
    default_options = {"protobuf/*:shared": False}

    def export_sources(self):
        copy(self, "conan-skywire-generate.cmake", self.recipe_folder, self.export_sources_folder)

    def layout(self):
        cmake_layout(self, src_folder="src")

    def requirements(self):
        self.requires("protobuf/7.36.2", visible=False)
        # Every generated C++ message includes protozero.
        self.requires("protozero/1.7.1", transitive_headers=True)

    def build_requirements(self):
        # The build runs protoc on skywire's own .proto: one that runs here,
        # whatever the host protobuf is (shared, or for another machine).
        self.tool_requires("protobuf/<host_version>")
        # skywire needs CMake 3.28; ConanCenter's build machines have older.
        self.tool_requires("cmake/[>=3.28 <5]")

    def package_id(self):
        # wiregen is a tool, and the runtimes are headers: the compiler that
        # built wiregen does not matter to consumers.
        del self.info.settings.compiler

    def validate(self):
        check_min_cppstd(self, 17)
        # wiregen runs in consumers' builds, where protobuf (a private
        # requirement) and its dependencies are not on the library path: link
        # them statically.
        for dep in self.dependencies.host.values():
            if dep.ref.name in ("protobuf", "abseil", "zlib") and dep.options.get_safe("shared"):
                raise ConanInvalidConfiguration(
                    f"{self.ref} needs {dep.ref.name}/*:shared=False: wiregen must "
                    "not depend on its shared libraries at run time")

    def source(self):
        get(self, **self.conan_data["sources"][self.version], strip_root=True)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.cache_variables["BUILD_TESTING"] = False
        tc.cache_variables["SKYWIRE_EXAMPLES"] = False
        tc.cache_variables["SKYWIRE_BENCH"] = False
        tc.cache_variables["SKYWIRE_INSTALL_DLLS"] = False
        tc.generate()
        CMakeDeps(self).generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(self, "LICENSE", self.source_folder, os.path.join(self.package_folder, "licenses"))
        cmake = CMake(self)
        cmake.install()
        rmdir(self, os.path.join(self.package_folder, "share", "doc"))
        rmdir(self, os.path.join(self.package_folder, "lib", "pkgconfig"))
        rm(self, "*.pdb", self.package_folder, recursive=True)
        # Conan writes the CMake config; keep skywire_generate() as a build
        # module, with the wiregen target and variables skywire's own config
        # would define.
        cmake_dir = os.path.join(self.package_folder, "lib", "cmake", "skywire")
        rm(self, "skywire*Config*.cmake", cmake_dir)
        rm(self, "skywireTargets*.cmake", cmake_dir)
        copy(self, "conan-skywire-generate.cmake", self.export_sources_folder, cmake_dir)

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "skywire")
        self.cpp_info.set_property("cmake_build_modules", [
            os.path.join("lib", "cmake", "skywire", "conan-skywire-generate.cmake")])

        c_runtime = self.cpp_info.components["c_runtime"]
        c_runtime.set_property("cmake_target_name", "skywire::c_runtime")
        c_runtime.includedirs = [os.path.join("include", "skywire")]
        c_runtime.libdirs = []
        # wiregen, on the PATH of the build and run environments.
        c_runtime.bindirs = ["bin"]

        cpp_runtime = self.cpp_info.components["cpp_runtime"]
        cpp_runtime.set_property("cmake_target_name", "skywire::cpp_runtime")
        cpp_runtime.includedirs = [os.path.join("include", "skywire")]
        cpp_runtime.libdirs = []
        cpp_runtime.bindirs = []
        cpp_runtime.requires = ["protozero::protozero"]
