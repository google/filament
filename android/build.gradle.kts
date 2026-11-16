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

plugins {
    alias(libs.plugins.nexus.publish)
    alias(libs.plugins.dokka) apply false
    id("com.google.android.filament.library") apply false
    id("com.google.android.filament.sample") apply false
    id("com.google.android.filament.publish") apply false
}

group = providers.gradleProperty("GROUP").get()
version = providers.gradleProperty("VERSION_NAME").get()

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
