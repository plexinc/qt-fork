@Library('plextools') import tv.plex.PlexTools
import groovy.json.JsonOutput

timestamps {

def dockerImage = "plex/build-server:latest"
def tools = new PlexTools(this)

def targets = ['plex-macos-x86_64-clang-libcxx-release',
               'plex-macos-x86_64-clang-libcxx-debug',
               'plex-windows-x86_64-msvc15-debug',
               'plex-windows-x86_64-msvc15-release']

stage("Build Qt") {
  builds = [:]
  targets.each { profileName ->
    builds[profileName] = {
      tools.autoDocker(profileName, dockerImage) {
        tools.uniqueWS(project: "qt-builds", profile: profileName) {
          checkout(tools.checkout_obj())

          def isMac = profileName.startsWith("plex-macos")
          def isWin = profileName.startsWith("plex-windows")
          def isDebug = profileName.endsWith("-debug")

          def skip_modules = [
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
            "qtbluetooth",
            "qtlocation",
            "qtscxml",
            "qtspeech",
            "qtwebgl",
            "qtlottie",
            "qtwebglplugin",
          ]

          def common_flags = [
            "-opensource",
            "-confirm-license",
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

          skip_modules.each { module ->
            common_flags.add("-skip ${module}")
          }

          if (isMac) {
            common_flags.addAll([
              "-securetransport",
              "-opengl desktop",
              "-sdk macosx10.14",
              "-separate-debug-info",
              "-device-option QMAKE_APPLE_DEVICE_ARCHS=x86_64",
              "-xplatform macx-clang",
            ])

            if (isDebug) {
              common_flags.add("-debug-and-release")
            }
          }

          if (isWin) {
            common_flags.addAll([
              "-schannel",
              "-opengl dynamic",
            ])

            if (isDebug) {
              common_flags.add("-debug")
            }
          }

          if (!isDebug) {
            common_flags.addAll(["-release", "-ltcg"])
          }

          dir("build") {
            def python_path = "c:\\Python27;" ? profileName.startsWith("plex-windows") : "/Users/jenkins/.pyenv/versions/2.7.16:"
            withEnv(["PATH=${python_path}${env.PATH}"]) {
              def flags = common_flags.join(" ")
              echo("Configure line: ${flags}")
              if (isMac) {
                tools.run(command: "../configure ${flags}")
              }
              else if (isWin) {
                tools.run(command: "..\\configure.bat ${flags}")
              }
            }
          }
        }
      }
    }
  }

  parallel builds

}

}