@Library('plextools') import tv.plex.PlexTools

timestamps {

def dockerImage = 'plex/build-server:latest'
def tools = new PlexTools(this)

def targets = [
  // Not the real profile names - this doesn't really matter for Qt because
  // it doesn't use the profiles. We do rely on the -release and -debug ending
  // in the build script.
  'plex-macos-x86_64-appleclang-toolchain-release',
  'plex-macos-x86_64-appleclang-toolchain-debug',
  //'plex-windows-x86_64-msvc15-debug',
  //'plex-windows-x86_64-msvc15-release'
]

// ----------------------------------------------------------------------------

def artifactoryServer = Artifactory.server 'ArtifactoryServer'
def buildInfo = Artifactory.newBuildInfo()

def uploadToArtifactory = {
  def uploadSpec = readFile("Artifactory.spec")
  echo uploadSpec
  def localBI = artifactoryServer.upload(spec: uploadSpec)
  buildInfo.append(localBI)
}

// ----------------------------------------------------------------------------

stage("Build Qt") {
  builds = [:]
  targets.each { profileName ->
    def realProfileName = profileName
    if (profileName.contains('macos')) {
      realProfileName = 'plex-macos-x86_64-appleclang-toolchain'
    }
    builds[profileName] = {
      tools.autoDocker(realProfileName, dockerImage) {
        tools.uniqueWS(project: "qt-builds", profile: profileName) {
          checkout(tools.checkout_obj())

          dir("build") {
            tools.run(command: "../plex/build.py --make-package ${profileName}")
            uploadToArtifactory()
          }
        }
      }
    }
  }

  parallel builds
}

stage("Publish Build Info") {
  node('master') {
    artifactoryServer.publishBuildInfo(buildInfo)
  }
}

}
