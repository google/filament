plugins {
    id("com.android.application")
    id("com.google.android.filament-tools")
}

project.extra["isSample"] = true

kotlin {
    jvmToolchain(libs.versions.jdk.get().toInt())
}

// The C++ samples request assets with the same relative paths as on desktop, where they are laid
// out by samples/CMakeLists.txt. AndroidAssetLoader strips the leading "assets/", so
// "assets/ibl/..." maps to "ibl/..." and "textures/..." stays as is in the APK.
val generatedAssetsDir = project.layout.projectDirectory.dir("src/main/assets")

// Generate the default IBL (assets/ibl/lightroom_14b) with the same cmgen arguments as desktop.
filament {
    iblInputFile.set(
        project.layout.projectDirectory.file(
            "../../../third_party/environments/lightroom_14b.hdr"
        )
    )
    iblOutputDir.set(generatedAssetsDir.dir("ibl"))
    cmgenArgs.set(
        "--quiet --format=ktx --size=256 --extract-blur=0.1 -x " +
            generatedAssetsDir.dir("ibl/lightroom_14b").asFile.absolutePath
    )
}

// Stage the textures under textures/, which is where the samples look for them.
val stageTextures = tasks.register<Sync>("stageSampleTextures") {
    from("../../../third_party/textures")
    into(generatedAssetsDir.dir("textures"))
    exclude("**/URL.txt", "**/CC0.html")
}
tasks.named("preBuild") {
    dependsOn(stageTextures)
}

tasks.named("clean") {
    doFirst {
        delete("src/main/assets")
    }
}

android {
    namespace = "com.google.android.filament.cppviewer"
    compileSdk = libs.versions.compileSdk.get().toInt()
    // The native code links against Filament's static libraries, so it must use the same NDK
    // (and therefore the same libc++) that build.sh used to compile them.
    ndkVersion = libs.versions.ndk.get()

    defaultConfig {
        applicationId = "com.google.android.filament.cppviewer"
        minSdk = libs.versions.minSdk.get().toInt()
        targetSdk = libs.versions.targetSdk.get().toInt()
        versionCode = 1
        versionName = "1.0"

        externalNativeBuild {
            cmake {
                arguments("-DFILAMENT_DIST_DIR=${rootProject.extra["filamentPath"]}")
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

    compileOptions {
        val jdkVersion = JavaVersion.toVersion(libs.versions.jdk.get())
        sourceCompatibility = jdkVersion
        targetCompatibility = jdkVersion
    }
}

dependencies {
    implementation(libs.kotlin.stdlib)
}
