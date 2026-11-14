plugins {
    id("com.google.android.filament.sample")
    id("com.google.android.filament-tools")
}

// The C++ samples request assets with the same relative paths as on desktop, where they are laid
// out by samples/CMakeLists.txt. AndroidAssetLoader strips the leading "assets/", so
// "assets/ibl/..." maps to "ibl/..." and "textures/..." stays as is in the APK.
val generatedAssetsDir = layout.buildDirectory.dir("generated/assets/filament")

// Generate the default IBL (assets/ibl/lightroom_14b) with the same cmgen arguments as desktop.
filament {
    iblInputFile.set(
        project.layout.projectDirectory.file(
            "../../../third_party/environments/lightroom_14b.hdr"
        )
    )
    iblOutputDir.set(generatedAssetsDir.map { it.dir("ibl") })
    cmgenArgs.set(
        generatedAssetsDir.map {
            "--quiet --format=ktx --size=256 --extract-blur=0.1 -x " +
                it.dir("ibl/lightroom_14b").asFile.absolutePath
        }
    )
}

// Stage the textures under textures/, which is where the samples look for them.
val stageTextures = tasks.register<Sync>("stageSampleTextures") {
    from("../../../third_party/textures")
    into(generatedAssetsDir.map { it.dir("textures") })
    exclude("**/URL.txt", "**/CC0.html")
}
tasks.named("filamentGenerateAssets") {
    dependsOn(stageTextures)
}

val distDirProp = providers.gradleProperty("com.google.android.filament.dist-dir").get()
val distFile = File(distDirProp).let {
    if (it.isAbsolute) it else File(rootDir, distDirProp)
}
val filamentPath = distFile.canonicalFile.absolutePath.replace(File.separator, "/")

android {
    namespace = "com.google.android.filament.cppviewer"
    // The native code links against Filament's static libraries, so it must use the same NDK
    // (and therefore the same libc++) that build.sh used to compile them.
    ndkVersion = libs.versions.ndk.get()

    defaultConfig {
        applicationId = "com.google.android.filament.cppviewer"
        versionCode = 1
        versionName = "1.0"

        externalNativeBuild {
            cmake {
                arguments("-DFILAMENT_DIST_DIR=$filamentPath")
            }
        }

        ndk {
            abiFilters += "arm64-v8a"
        }
    }

    externalNativeBuild {
        cmake {
            version = libs.versions.cmake.get()
            path = file("CMakeLists.txt")
        }
    }

    androidResources {
        ignoreAssetsPattern = "!.svn:!.git:!*.txt:!*.html:!*.md"
    }

    sourceSets {
        getByName("main") {
            assets.directories.addAll(
                listOf(
                    "../../../assets",
                    "../../../third_party/models"
                )
            )
            (assets as org.gradle.api.tasks.util.PatternFilterable).exclude(
                "**/URL.txt",
                "**/CC0.html",
                "URL.txt",
                "CC0.html"
            )
        }
    }
}

dependencies {
    implementation(libs.kotlin.stdlib)
}
