#! /usr/bin/env python3

import platform
import os
import subprocess as sp
import shutil

from pathlib import Path
from io import StringIO


SCRIPT_PATH = Path(__file__).parent.resolve()
BUILD_ROOT = SCRIPT_PATH.parent.resolve()


class Build:
  skip_modules = [
    "qt3d",
    "qtdoc",
    "qtmultimedia",
    "qtsensors",
    "qtserialport",
    "qtactiveqt",
    "qtcharts",
    "qtdatavis3d",
    "qtgraphicaleffects",
    "qtpurchasing",
    "qtwebview",
    "qtscript",
    "qtlocation",
    "qtscxml",
    "qtspeech",
    "qtlottie",
    "qtwebglplugin",
  ]

  common_flags = [
    "-opensource",
    "-confirm-license",
    "-webengine-proprietary-codecs",
    "-nomake tests",
    "-nomake examples",
    "-no-gif",
    "-qt-libpng",
    "-qt-libjpeg",
    "-qt-pcre",
    "-no-cups",
    "-no-dbus",
    "-pch",
    "-no-qml-debug",
    "-no-openssl",
  ]

  def __init__(self, profile: str):
    self.profile = profile

  def run(self):
    try:
      cwd = os.getcwd()
      os.chdir(self.script_path)
      if self.is_windows:
        return self.run_windows()
      elif self.is_macos:
        return self.run_macos()
    finally:
      os.chdir(cwd)

  def _download_jom(self):
    # we have curl on the build nodes, but not requests (it's also quicker)
    sp.run(["curl", "-L", "-o", "jom.zip",
            "http://download.qt.io/official_releases/jom/jom.zip"])
    from zipfile import ZipFile
    with ZipFile("jom.zip", "r") as zfp:
      zfp.extractall()
    os.remove("jom.zip")

  def run_windows(self):
    self._download_jom()
    # to make the path as short as possible we create a junction here.
    rootdrive = os.path.splitdrive(os.getenv("JENKINS_WORKSPACE_ROOT", "c:\\"))[0]
    root_junction = Path(f"{rootdrive}\\j")
    root_junction.mkdir(exist_ok=True)
    junctiondir = root_junction / os.getenv("PLEX_BUILD_HASH", "xx")
    if junctiondir.exists():
      # junctions? more like junk-tions
      sp.run(["fsutil", "reparsepoint", "delete", str(junctiondir)], shell=True)
      try:
        shutil.rmtree(junctiondir)
      except FileNotFoundError:
        pass
    mklink = sp.run(["mklink", "/J", str(junctiondir), str(self.build_root)], shell=True)

    flags = self.compute_flags()
    env = self.compute_env()
    for extra_path in ("gnuwin/bin", "qtbase/bin", str(self.script_path)):
      env["PATH"] = f"{env['PATH']}{os.pathsep}{str(self.build_root / extra_path)}"
    env["PYTHONHOME"] = self.python_path

    with StringIO() as script:
      for key, value in env.items():
        print(f"set {key}={value}", file=script)
      print()

      print(f"cd {self.build_root}", file=script)
      print('call "C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Community\\VC\\Auxiliary\\Build\\vcvarsall.bat" amd64', file=script)
      print(f"call {str(self.build_root / 'configure.bat')} {' '.join(flags)}", file=script)
      print(f"jom", file=script)
      print(script.getvalue())
      build_script = Path("plex_build.cmd")
      with build_script.open("w") as fp:
        fp.write(script.getvalue())
      input()
      sp.run(["cmd", "/C", str(build_script.resolve())])

  def run_macos(self):
    pass

  @property
  def is_macos(self):
    return platform.system() == "Darwin"

  @property
  def is_windows(self):
    return platform.system() == "Windows"

  @property
  def is_debug(self):
    return self.profile.endswith("-debug")

  @property
  def build_root(self):
    return BUILD_ROOT

  @property
  def python_path(self):
    if self.is_windows:
      return "C:\\Python27"
    elif self.is_macos:
      raise RuntimeError("Not implemented")

  @property
  def script_path(self):
    return SCRIPT_PATH

  def compute_flags(self) -> str:
    flags = self.common_flags
    flags += [f"-skip {mod}" for mod in self.skip_modules]

    if self.is_macos:
      flags += [
        "-securetransport",
        "-opengl desktop",
        "-sdk macosx10.14",
        "-separate-debug-info",
        "-device-option QMAKE_APPLE_DEVICE_ARCHS=x86_64",
        "-xplatform macx-clang",
      ]
      if self.is_debug:
        flags += ["-debug-and-release"]
    elif self.is_windows:
      flags += [ "-schannel", "-opengl dynamic" ]
      if self.is_debug:
        flags += ["-debug"]

    if not self.is_debug:
      flags += ["-release", "-ltcg"]

    return flags

  def compute_env(self):
    jobs = os.getenv("PLEX_JOBS", "6")
    environment = {
      "CFLAGS": "", "CXXFLAGS": "", "LDFLAGS": "", "CL": "",
      "NINJAFLAGS": f"-j{jobs} -v",
      "PATH": f"{self.python_path}{os.pathsep}{os.environ['PATH']}"
    }
    return environment


if __name__ == "__main__":
  from argparse import ArgumentParser
  parser = ArgumentParser()
  parser.add_argument("profile")
  args = parser.parse_args()
  build = Build(args.profile)
  build.run()