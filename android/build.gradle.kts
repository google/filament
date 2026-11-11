// This script accepts the following parameters:
//
// com.google.android.filament.dist-dir
//     Path to the Filament distribution/install directory for Android
//     (produced by make/ninja install). This directory must contain lib/arm64-v8a/ etc.
//
// com.google.android.filament.tools-dir
//     Path to the Filament distribution/install directory for desktop.
//     This directory must contain bin/matc.
//
// com.google.android.filament.exclude-vulkan
//     When set, support for Vulkan will be excluded.
//
// com.google.android.filament.include-webgpu
//     When set, enables WebGPU support.
//
// com.google.android.filament.fgviewer
//     When set, enables fgviewer
//
// com.google.android.filament.matdbg
//     When set, enables matdbg
//
// com.google.android.filament.matnopt
//     When set, disable shader optimizations.
//
// com.google.android.filament.skip-samples
//     Exclude samples from the project. Useful to speed up compilation.
//
// com.google.android.filament.abis
//     List of supported ABIs to build as a comma separated list. Available options are:
//         arm64-v8a, armeabi-v7a, x86_64, x86, all
//     Defaults to all.
//
// Example:
//     ./gradlew -Pcom.google.android.filament.dist-dir=../dist-android-release \
//               -Pcom.google.android.filament.abis=x86 \
//               assembleRelease

// Publishing to Maven Central:
//   - Build and upload artifacts with ./gradlew publish
//   - Close and release staging repo on Nexus with ./gradlew closeAndReleaseStagingRepository
//
// The following properties need to be set (either in ~/gradle/gradle.properties, on the command
// line, or as environment variables, e.g.: ORG_GRADLE_PROJECT_property=value):
//
//     sonatypeUsername=nexus_user
//     sonatypePassword=nexus_password
//
//     To sign with a key ring file:
//     signing.keyId=pgp_key_id
//     signing.password=pgp_key_password
//     signing.secretKeyRingFile=/Users/user/.gnupg/maven_signing.key
//
//     To sign with in-memory keys (useful for CI):,
//     signingKey=ASCII armored key (begins with -----BEGIN PGP PRIVATE KEY BLOCK-----)
//     signingPassword=key password
//

import com.android.build.api.dsl.LibraryExtension

buildscript {
    val path = providers
        .gradleProperty("com.google.android.filament.dist-dir")
        .get()

    val directory = objects.fileProperty().fileValue(File(path)).asFile.get()
    var filamentPath = directory.absolutePath

    // Our CMake scripts require a forward-slash path for the FILAMENT_DIST_DIR
    // variable, so here we convert the native path to a forward-slash path.
    filamentPath = filamentPath.replace(File.separator, "/")
    extra["filamentPath"] = filamentPath

    // Warning: changing this property does not work well with incremental builds.
    val excludeVulkan = providers
        .gradleProperty("com.google.android.filament.exclude-vulkan")
        .isPresent

    val includeWebGPU = providers
        .gradleProperty("com.google.android.filament.include-webgpu")
        .isPresent

    val fgviewer = providers
        .gradleProperty("com.google.android.filament.fgviewer")
        .isPresent

    val matdbg = providers
        .gradleProperty("com.google.android.filament.matdbg")
        .isPresent

    val matnopt = providers
        .gradleProperty("com.google.android.filament.matnopt")
        .isPresent

    val mutexdebug = providers
        .gradleProperty("com.google.android.filament.mutexdebug")
        .isPresent

    var abis = listOf("arm64-v8a", "armeabi-v7a", "x86_64", "x86")
    val newAbis = providers
        .gradleProperty("com.google.android.filament.abis")
        .get()
        .split(",")
    if (!newAbis.contains("all")) {
        abis = newAbis
    }

    dependencies {
        classpath(libs.android.gradle.plugin)
        classpath(libs.kotlin.gradle.plugin)
    }

    extra["cmakeArgs"] = listOf(
        "--no-warn-unused-cli",
        "-DANDROID_WEAK_API_DEFS=ON",
        "-DANDROID_PIE=ON",
        "-DANDROID_PLATFORM=21",
        "-DANDROID_STL=c++_static",
        "-DFILAMENT_DIST_DIR=$filamentPath",
        "-DFILAMENT_SUPPORTS_VULKAN=${if (excludeVulkan) "OFF" else "ON"}",
        "-DFILAMENT_SUPPORTS_WEBGPU=${if (includeWebGPU) "ON" else "OFF"}",
        "-DFILAMENT_ENABLE_FGVIEWER=${if (fgviewer) "ON" else "OFF"}",
        "-DFILAMENT_ENABLE_MATDBG=${if (matdbg) "ON" else "OFF"}",
        "-DFILAMENT_DISABLE_MATOPT=${if (matnopt) "ON" else "OFF"}",
        "-DFILAMENT_DEBUG_MUTEX=${if (mutexdebug) "ON" else "OFF"}"
    )

    extra["cppFlags"] = listOf(
        "-std=c++17",
        "-fno-stack-protector",
        "-fno-unwind-tables",
        "-fno-asynchronous-unwind-tables",
        "-fno-rtti",
        "-ffast-math",
        "-fno-finite-math-only",
        "-ffp-contract=fast",
        "-fvisibility-inlines-hidden",
        "-fvisibility=hidden",
        "-fomit-frame-pointer",
        "-ffunction-sections",
        "-fdata-sections",
        "-no-canonical-prefixes",
        "-Wformat",
        "-Werror=format-security",
        "-Wno-unused-command-line-argument",
        "-Wl,--gc-sections",
        "-Wl,-Bsymbolic-functions",
        "-Wl,--hash-style=both" // Required to support API levels below 23
    )

    extra["abis"] = abis

    repositories {
        mavenCentral()
        google()
    }
}

plugins {
    alias(libs.plugins.nexus.publish)
}

// See https://github.com/gradle-nexus/publish-plugin
// Publish to https://oss.sonatype.org/ (not s01)
nexusPublishing {
    packageGroup.set("com.google.android")
    repositories {
        sonatype {
            stagingProfileId.set("9a75a224a4f17b")
            nexusUrl.set(uri("https://ossrh-staging-api.central.sonatype.com/service/local/"))
            snapshotRepositoryUrl.set(uri("https://central.sonatype.com/repository/maven-snapshots/"))
        }
    }
}

val versionCatalog = libs

@Suppress("UNCHECKED_CAST")
val rootCmakeArgs = rootProject.extra["cmakeArgs"] as List<String>
@Suppress("UNCHECKED_CAST")
val rootCppFlags = rootProject.extra["cppFlags"] as List<String>
@Suppress("UNCHECKED_CAST")
val rootAbis = rootProject.extra["abis"] as List<String>

subprojects {
    group = providers.gradleProperty("GROUP").get()
    version = providers.gradleProperty("VERSION_NAME").get()

    repositories {
        mavenCentral()
        google()
    }

    if (!name.startsWith("sample") && name != "filament-tools" && name != "gradle-plugin") {
        apply(plugin = "com.android.library")

        extensions.configure<LibraryExtension> {
            buildToolsVersion = versionCatalog.versions.buildTools.get()
            compileSdk = versionCatalog.versions.compileSdk.get().toInt()
            ndkVersion = versionCatalog.versions.ndk.get()

            defaultConfig {
                minSdk = versionCatalog.versions.minSdk.get().toInt()

                externalNativeBuild {
                    cmake {
                        arguments.addAll(rootCmakeArgs)
                        cppFlags.addAll(rootCppFlags)
                    }
                }

                ndk {
                    //noinspection ChromeOsAbiSupport
                    abiFilters.addAll(rootAbis)
                }

                buildFeatures {
                    buildConfig = true
                }

                consumerProguardFiles("../proguard-rules.pro")
            }

            externalNativeBuild {
                cmake {
                    version = versionCatalog.versions.cmake.get()
                    path = file("CMakeLists.txt")
                }
            }

            compileOptions {
                val jdkVersion = JavaVersion.toVersion(versionCatalog.versions.jdk.get())
                sourceCompatibility = jdkVersion
                targetCompatibility = jdkVersion
            }
        }
    }
}

gradle.taskGraph.whenReady {
    allTasks.forEach { task ->
        task.onlyIf {
            !task.project.extra.has("isSample") ||
                !project.hasProperty("com.google.android.filament.skip-samples")
        }
    }
}
