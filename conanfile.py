from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain, cmake_layout

class RtsEngineConan(ConanFile):
    name = "rts_engine"
    version = "0.1.0"
    settings = "os", "compiler", "build_type", "arch"

    options = {
        "build_server": [True, False],
        "build_client": [True, False],
        "build_tests":  [True, False],
    }
    default_options = {
        "build_server": True,
        "build_client": True,
        "build_tests":  True,
        # Opcje bibliotek (zredukowane do najbardziej istotnych)
        "boost/*:header_only": False,
        "flatbuffers/*:flatc": True,
    }

    def requirements(self):
        self.requires("entt/3.13.2")
        self.requires("boost/1.85.0")
        self.requires("enet/1.3.17")
        self.requires("flatbuffers/24.3.25")
        self.requires("libpqxx/8.0.1")
        self.requires("spdlog/1.14.1")
        self.requires("sfml/3.0.2") 
        self.requires("imgui/1.91.8")
        self.requires("imgui-sfml/3.0")
        self.requires("prometheus-cpp/1.2.4")

    def build_requirements(self):
        if self.options.build_tests:
            self.test_requires("catch2/3.7.1")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        
        tc = CMakeToolchain(self)
        tc.user_presets_path = False 
        tc.cache_variables["RTS_BUILD_SERVER"] = bool(self.options.build_server)
        tc.cache_variables["RTS_BUILD_CLIENT"] = bool(self.options.build_client)
        tc.cache_variables["RTS_BUILD_TESTS"]  = bool(self.options.build_tests)
        tc.generate()