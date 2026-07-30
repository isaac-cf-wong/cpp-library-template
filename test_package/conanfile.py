"""Conan test package: proves a published package is actually consumable.

``conan create .`` builds the library and then builds and runs this project
against the packaged result. It is the Conan-side counterpart of the
``package_consumption`` CTest test, which covers the same ground for plain CMake
consumers.
"""

from __future__ import annotations

import os

from conan import ConanFile
from conan.tools.build import can_run
from conan.tools.cmake import CMake, cmake_layout


class CppLibraryTemplateTestConan(ConanFile):
    """Minimal downstream consumer of the package under test."""

    settings = "os", "arch", "compiler", "build_type"
    generators = "CMakeDeps", "CMakeToolchain"
    test_type = "explicit"

    def requirements(self) -> None:
        """Depend on the package being tested."""
        self.requires(self.tested_reference_str)

    def layout(self) -> None:
        """Use the standard CMake layout."""
        cmake_layout(self)

    def build(self) -> None:
        """Build the consumer."""
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def test(self) -> None:
        """Run the consumer and let a non-zero exit status fail the test."""
        if can_run(self):
            self.run(os.path.join(self.cpp.build.bindir, "consumer"), env="conanrun")
