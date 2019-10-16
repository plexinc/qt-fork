#! /usr/bin/env python3

import platform
import os
import re
import subprocess as sp
import shutil

from pathlib import Path
from io import StringIO
from contextlib import contextmanager


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

  prefix = "qt-install"

  def __init__(self, profile: str):
    self.profile = profile
    self.common_flags.append(f"-prefix {str(self.build_root / self.prefix)}")

  def run(self, is_dry=False):
    flags = self.compute_flags()
    env = self.compute_env()
    if self.is_windows:
      return self.run_windows(is_dry, flags, env)
    elif self.is_macos:
      return self.run_macos(is_dry, flags, env)

  def run_windows(self, is_dry, flags, env):
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

    paths = [self.python_path, *self._sanitize_path()]
    for extra_path in ("gnuwin/bin", "qtbase/bin", str(self.script_path)):
      paths.append(str(self.build_root / extra_path))
    env["PATH"] = os.pathsep.join(paths)
    env["PYTHONHOME"] = self.python_path

    with StringIO() as script:
      for key, value in env.items():
        print(f"set {key}={value}", file=script)
      print(file=script)

      vs_dir = self._get_vs_dir()
      print(f"python -V", file=script)
      print(f'call "{vs_dir}\\VC\\Auxiliary\\Build\\vcvarsall.bat" amd64', file=script)
      print(f"call {str(self.build_root / 'configure.bat')} {' '.join(flags)}", file=script)
      print(f"jom /j{self.jobs}", file=script)
      print(f"if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%", file=script)
      print(f"jom install", file=script)
      print(f"if %ERRORLEVEL% GEQ 1 EXIT /B %ERRORLEVEL%", file=script)
      print(script.getvalue())
      build_script = Path("plex_build.cmd")
      with build_script.open("w") as fp:
        fp.write(script.getvalue())
      if not is_dry:
        sp.run(["cmd", "/C", str(build_script.resolve())]).check_returncode()
      else:
        print(script.getvalue())

  def run_macos(self, is_dry, flags, env):
    with StringIO() as script:
      print("#! /bin/bash", file=script)
      print("set +x", file=script)
      print("set +v", file=script)
      print(file=script)
      for key, value in env.items():
        print(f"export {key}='{value}'", file=script)
      print(file=script)
      print(f"python -V", file=script)
      print("./configure", file=script)
      print(f"./make -j {self.jobs}", file=script)
      print(f"./make install", file=script)
      build_script = Path("plex_build.sh")
      with build_script.open("w") as fp:
        fp.write(script.getvalue())
      import stat
      st = os.stat(build_script).st_mode
      os.chmod(build_script, st | stat.S_IEXEC)
      print(script.getvalue())
      if not is_dry:
        sp.run(["bash", "-c", str(build_script.resolve())]).check_returncode()



  def package(self):
    with chdir(self.build_root / self.prefix):
      print(f"Creating {self.package_name}")
      cmake = sp.run(["cmake", "-E", "tar", "cJf",
                      f"../{self.package_name}",
                      "--format=gnutar", "."])
      cmake.check_returncode()

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
    jobs = self.jobs
    environment = {
      "CFLAGS": "", "CXXFLAGS": "", "LDFLAGS": "", "CL": "",
      "NINJAFLAGS": f"-j{jobs} -v",
      "PATH": f"{self.python_path}{os.pathsep}{os.environ['PATH']}"
    }
    return environment

  def write_spec(self):
    template = open(self.script_path / "Artifactory.spec.in").read()
    os_name = {
      "Darwin": "Macos",
      "Windows": "Windows",
      "Linux": "Linux"
    }[platform.system()]
    subst = {
      "build_root": str(self.build_root).replace("\\", "/"),
      "package_name": self.package_name,
      "qt_version": self.qt_version,
      "git_sha": self.git_sha,
      "full_version": self.full_version,
      "build_type": self.build_type,
      "os_name": os_name,
    }
    with open("Artifactory.spec", "w") as spec:
      spec.write(template.format(**subst))

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
  def build_type(self):
    return "debug" if self.is_debug else "release"

  @property
  def build_root(self):
    return BUILD_ROOT

  @property
  def python_path(self):
    if self.is_windows:
      return "C:\\Python27"
    elif self.is_macos:
      if Path("/usr/bin/python2").exists():
        # catalina
        return "/usr/bin/python2"
      else:
        return "/usr/bin/python"

  @property
  def script_path(self):
    return SCRIPT_PATH

  @property
  def jobs(self):
    return os.getenv("PLEX_JOBS", "6")

  @property
  def qt_version(self):
    with open(self.build_root / "qtbase/.qmake.conf") as qconfig:
      for line in qconfig:
        match = re.search(r'MODULE_VERSION = (.+)', line)
        if match:
          return match.group(1)
    raise RuntimeError("Could not read MODULE_VERSION from qtbase/.qmake.conf")

  @property
  def full_version(self):
    return f"{self.qt_version}-{self.git_sha[:8]}"

  @property
  def git_sha(self):
    if "GIT_COMMIT" in os.environ:
      sha = os.environ["GIT_COMMIT"]
    else:
      git = sp.run(["git", "rev-parse", "HEAD"], stdout=sp.PIPE)
      git.check_returncode()
      sha = git.stdout.decode().strip()
    return sha

  @property
  def package_name(self):
    os_name = platform.system().lower()
    return f"qt-{self.full_version}-{os_name}-x86_64-{self.build_type}.tar.xz"

  def _download_jom(self):
    # we have curl on the build nodes, but not requests (it's also quicker)
    sp.run(["curl", "-L", "-o", "jom.zip",
            "http://download.qt.io/official_releases/jom/jom.zip"])
    from zipfile import ZipFile
    with ZipFile("jom.zip", "r") as zfp:
      zfp.extractall()
    os.remove("jom.zip")

  def _get_vs_dir(self):
    import json
    vswhere = sp.run([str(self.build_root / "vswhere.exe"), "-format", "json",
                      "-version", "15.0"], stdout=sp.PIPE)
    vswhere.check_returncode()
    vs_data = json.loads(vswhere.stdout.decode())
    return vs_data[0]["installationPath"]

  def _sanitize_path(self):
    paths = os.environ["PATH"].split(os.pathsep)
    result = []
    for path in paths:
      if "python" in path.lower():
        continue
      result.append(path)
    return result


@contextmanager
def chdir(dirname):
  try:
    cwd = os.getcwd()
    os.chdir(dirname)
    yield
  finally:
    os.chdir(cwd)


if __name__ == "__main__":
  from argparse import ArgumentParser
  parser = ArgumentParser()
  parser.add_argument("--dry-run", action="store_true", help="Only display steps")
  parser.add_argument("--make-package", action="store_true", help="Create tarball")
  parser.add_argument("profile")
  args = parser.parse_args()
  build = Build(args.profile)
  build.run(args.dry_run)
  if args.make_package:
    build.package()
  build.write_spec()

