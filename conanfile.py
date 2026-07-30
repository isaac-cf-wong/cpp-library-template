"""Conan 2 recipe for cpp_library_template.

This is the C++ analogue of ``pyproject.toml``: it declares the package
metadata, the dependencies, and how to build and package the library.

The recipe is optional for developing the project. ``cmake --workflow --preset
dev`` works with no Conan installed at all, because ``tests/CMakeLists.txt``
falls back to FetchContent for GoogleTest. Conan is what makes the library
*consumable* and *publishable*::

    conan create . --build=missing          # build and run the test package
    conan upload cpp_library_template/<v> -r <remote> --confirm

The version is taken from the git tag, exactly like the CMake build, so it is
never written down twice.
"""

from __future__ import annotations

import os
import re
import shutil
import subprocess

from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.files import copy, load, save

required_conan_version = ">=2.0"


class CppLibraryTemplateConan(ConanFile):
    """Recipe for the library built by this repository."""

    name = "cpp_library_template"
    package_type = "library"

    license = "BSD-3-Clause"
    author = "Isaac C. F. Wong"
    url = "https://github.com/isaac-cf-wong/cpp-library-template"
    homepage = "https://github.com/isaac-cf-wong/cpp-library-template"
    description = "A GitHub template repository for a C++ library"
    topics = ("cpp", "cmake", "conan", "github", "template")

    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_cli": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "with_cli": False,
    }

    implements = ("auto_shared_fpic",)

    exports_sources = (
        "CMakeLists.txt",
        "CMakePresets.json",
        "LICENSE",
        "apps/*",
        "cmake/*",
        "include/*",
        "src/*",
        "tests/*",
    )

    def set_version(self) -> None:
        """Take the version from the git tag, or from the exported metadata.

        In a git checkout this runs ``git describe``. Inside the Conan cache
        there is no ``.git``, so the value recorded at export time is reused.
        """
        if self.version:
            return

        recorded = os.path.join(self.recipe_folder, "version.txt")
        if os.path.exists(recorded):
            self.version = load(self, recorded).strip()
            return

        git = shutil.which("git")
        if git is None:
            self.output.warning("git not found, using version 0.0.0")
            self.version = "0.0.0"
            return

        try:
            described = (
                subprocess.check_output(  # noqa: S603 -- a fixed argument list
                    [git, "describe", "--tags", "--abbrev=0", "--match", "v[0-9]*"],
                    cwd=self.recipe_folder,
                    stderr=subprocess.DEVNULL,
                )
                .decode()
                .strip()
            )
        except (OSError, subprocess.CalledProcessError):
            self.output.warning("no git tag found, using version 0.0.0")
            self.version = "0.0.0"
            return

        self.version = re.sub(r"^v", "", described)

    def export(self) -> None:
        """Record the resolved version so cache builds agree with the checkout.

        There is no ``.git`` inside the Conan cache, and CMake would otherwise
        fall back to 0.0.0 while the package reference said something else.
        """
        save(self, os.path.join(self.export_folder, "version.txt"), f"{self.version}\n")

    def requirements(self) -> None:
        """Declare runtime dependencies.

        The library deliberately has none. Add them here *and* make the CMake
        build find them; ``cmake/cpp_library_templateConfig.cmake.in`` must gain
        a matching ``find_dependency`` call for any dependency that appears in a
        public header.
        """

    def build_requirements(self) -> None:
        """Declare tooling and test-only dependencies."""
        self.test_requires("gtest/1.17.0")

    def validate(self) -> None:
        """Reject configurations the sources cannot support."""
        check_min_cppstd(self, 20)

        if self.settings.os == "Windows" and self.options.shared and self.settings.compiler == "gcc":
            raise ConanInvalidConfiguration("a shared build with MinGW is not supported")

    def layout(self) -> None:
        """Use the standard CMake layout so build folders stay out of the tree."""
        cmake_layout(self)

    def generate(self) -> None:
        """Write the CMake toolchain and dependency files."""
        deps = CMakeDeps(self)
        deps.generate()

        toolchain = CMakeToolchain(self)
        toolchain.cache_variables["CPP_LIBRARY_TEMPLATE_VERSION_OVERRIDE"] = self.version
        toolchain.cache_variables["CPP_LIBRARY_TEMPLATE_BUILD_CLI"] = bool(self.options.with_cli)
        # Conan builds the library itself; the test package covers consumption.
        toolchain.cache_variables["CPP_LIBRARY_TEMPLATE_BUILD_TESTS"] = not self.conf.get(
            "tools.build:skip_test", default=False, check_type=bool
        )
        toolchain.generate()

    def build(self) -> None:
        """Configure, build and test."""
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        if not self.conf.get("tools.build:skip_test", default=False, check_type=bool):
            # The package consumption test installs into its own prefix and is
            # redundant here: test_package/ covers the same ground the way Conan
            # consumers actually build.
            cmake.ctest(cli_args=["--label-exclude", "integration"])

    def package(self) -> None:
        """Install the built artefacts into the package folder."""
        cmake = CMake(self)
        cmake.install()
        copy(
            self,
            "LICENSE",
            src=self.source_folder,
            dst=os.path.join(self.package_folder, "licenses"),
        )

    def package_info(self) -> None:
        """Describe the package to consumers."""
        self.cpp_info.libs = ["cpp_library_template"]

        # Let consumers keep using find_package(cpp_library_template) and the
        # namespaced target, whether they came through Conan or not.
        self.cpp_info.set_property("cmake_file_name", "cpp_library_template")
        self.cpp_info.set_property("cmake_target_name", "cpp_library_template::cpp_library_template")

        if not self.options.shared:
            self.cpp_info.defines.append("CPP_LIBRARY_TEMPLATE_STATIC_DEFINE")

        if self.options.with_cli:
            self.cpp_info.bindirs = ["bin"]
