#############################################################################
##
## Copyright (C) 2021 The Qt Company Ltd.
## Contact: https://www.qt.io/licensing/
##
## This file is part of the release tools of the Qt Toolkit.
##
## $QT_BEGIN_LICENSE:GPL-EXCEPT$
## Commercial License Usage
## Licensees holding valid commercial Qt licenses may use this file in
## accordance with the commercial license agreement provided with the
## Software or, alternatively, in accordance with the terms contained in
## a written agreement between you and The Qt Company. For licensing terms
## and conditions see https://www.qt.io/terms-conditions. For further
## information use the contact form at https://www.qt.io/contact-us.
##
## GNU General Public License Usage
## Alternatively, this file may be used under the terms of the GNU
## General Public License version 3 as published by the Free Software
## Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
## included in the packaging of this file. Please review the following
## information to ensure the GNU General Public License requirements will
## be met: https://www.gnu.org/licenses/gpl-3.0.html.
##
## $QT_END_LICENSE$
##
#############################################################################

from conans import ConanFile
import os
import re
from functools import lru_cache
from pathlib import Path


@lru_cache(maxsize=8)
def _parse_qt_version_by_key(key: str) -> str:
    with open(Path(Path(__file__).parent.resolve() / ".cmake.conf")) as f:
        ret = [m.group(1) for m in [re.search(r"{0} .*\"(.*)\"".format(key), f.read())] if m]
    return ret.pop() if ret else ""


def _get_qt_minor_version() -> str:
    return _parse_qt_version_by_key('QT_REPO_MODULE_VERSION')[0:3]


class QtShadertools(ConanFile):
    name = "qtshadertools"
    license = "GPL-3.0+, Commercial Qt License Agreement"
    author = "The Qt Company <https://www.qt.io/contact-us>"
    url = "https://code.qt.io/cgit/qt/qtshadertools.git/"
    description = (
        "The Qt Shader Tools module builds on the SPIR-V Open Source Ecosystem as "
        "described at the Khronos SPIR-V web site"
    )
    topics = ("qt", "qt6", "qtshadertools", "SPIRV")
    settings = "os", "compiler", "arch", "build_type"
    exports = ".cmake.conf"  # for referencing the version number and prerelease tag
    exports_sources = "*", "!conan*.*"
    # use commit ID as the RREV (recipe revision) if this is exported from .git repository
    revision_mode = "scm" if Path(Path(__file__).parent.resolve() / ".git").exists() else "hash"
    python_requires = f"qt-conan-common/{_get_qt_minor_version()}@qt/everywhere"

    def set_version(self):
        _ver = _parse_qt_version_by_key("QT_REPO_MODULE_VERSION")
        _prerelease = _parse_qt_version_by_key("QT_REPO_MODULE_PRERELEASE_VERSION_SEGMENT")
        self.version = _ver + "-" + _prerelease if _prerelease else _ver

    def requirements(self):
        _version = _parse_qt_version_by_key("QT_REPO_MODULE_VERSION")
        # will match latest prerelase of final major.minor.patch
        self.requires(f"qtbase/[<={_version}, include_prerelease=True]@{self.user}/{self.channel}")

    def build(self):
        self.python_requires["qt-conan-common"].module.build_leaf_qt_module(self)

    def package(self):
        cmd = ["cmake", "--install", "."]
        self.run(" ".join(cmd))

    def package_info(self):
        self.python_requires["qt-conan-common"].module.package_info(self)

    def package_id(self):
        self.info.requires.package_revision_mode()

    def deploy(self):
        self.copy("*")  # copy from current package
        if not os.environ.get("QT_CONAN_INSTALL_SKIP_DEPS"):
            self.copy_deps("*")  # copy from dependencies
