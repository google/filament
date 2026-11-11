import de.undercouch.gradle.tasks.download.Download
import org.gradle.api.file.RelativePath

plugins {
    alias(libs.plugins.undercouch.download)
    `maven-publish`
}

apply(from = rootProject.file("gradle/gradle-mvn-push.gradle.kts"))

val versionName = providers.gradleProperty("VERSION_NAME").get()
val tools = listOf("matc", "cmgen")

data class PlatformConfig(
    val classifier: String,
    val archive: String,
    val path: (String) -> String
)

val platforms = mapOf(
    "mac" to PlatformConfig("osx-aarch_64", "filament-v$versionName-mac.tgz") { t -> "filament/bin/$t" },
    "linux" to PlatformConfig("linux-x86_64", "filament-v$versionName-linux.tgz") { t -> "filament/bin/$t" },
    "windows" to PlatformConfig("windows-x86_64", "filament-v$versionName-windows.tgz") { t -> "bin/$t.exe" }
)

platforms.forEach { (platform, config) ->
    val platformName = platform.replaceFirstChar { it.uppercase() }
    val remoteUrl = "https://github.com/google/filament/releases/download/v$versionName/${config.archive}"
    val downloadFile = layout.buildDirectory.file("downloads/${config.archive}")
    val extractDir = layout.buildDirectory.dir("extracted/filament-v$versionName-$platform")

    val downloadTask = tasks.register<Download>("downloadRelease$platformName") {
        src(remoteUrl)
        dest(downloadFile)
        overwrite(false)
    }

    tasks.register<Copy>("extractTools$platformName") {
        dependsOn(downloadTask)
        group = "setup"
        from(tarTree(resources.gzip(downloadFile)))

        // Include specific tools based on platform pattern
        include(tools.map { tool -> config.path(tool) })

        // Flatten the path so it lands directly in 'into'
        eachFile {
            relativePath = RelativePath(true, name)
        }

        into(extractDir)
        includeEmptyDirs = false
    }
}

publishing {
    publications {
        tools.forEach { toolName ->
            create<MavenPublication>(toolName) {
                artifactId = toolName

                platforms.forEach { (platform, config) ->
                    val platformName = platform.replaceFirstChar { it.uppercase() }
                    val extractDir = layout.buildDirectory.dir("extracted/filament-v$versionName-$platform").get().asFile
                    val archivePath = config.path(toolName)
                    val exeName = File(archivePath).name

                    artifact(File(extractDir, exeName)) {
                        classifier = config.classifier
                        extension = "exe"
                        builtBy(tasks.named("extractTools$platformName"))
                    }
                }
            }
        }
    }
}
