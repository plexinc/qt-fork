@Library('plextools') import tv.plex.PlexTools

timestamps {

def dockerImage = 'plex/build-server:latest'
def tools = new PlexTools(this)

def targets = [
  'plex-macos-x86_64-appleclang-toolchain',
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
    builds[profileName] = {
      tools.autoDocker(profileName, dockerImage) {
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
