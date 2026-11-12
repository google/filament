plugins {
    id("com.android.application")
}

project.extra["isSample"] = true

kotlin {
    jvmToolchain(libs.versions.jdk.get().toInt())
}

android {
    namespace = "com.google.android.filament.livewallpaper"

    compileSdk = libs.versions.compileSdk.get().toInt()
    defaultConfig {
        applicationId = "com.google.android.filament.livewallpaper"
        minSdk = libs.versions.minSdk.get().toInt()
        targetSdk = libs.versions.targetSdk.get().toInt()
    }

    // NOTE: This is a workaround required because the AGP task collectReleaseDependencies
    //       is not configuration-cache friendly yet; this is only useful for Play publication
    dependenciesInfo {
        includeInApk = false
    }

    compileOptions {
        val jdkVersion = JavaVersion.toVersion(libs.versions.jdk.get())
        sourceCompatibility = jdkVersion
        targetCompatibility = jdkVersion
    }
}

dependencies {
    implementation(libs.kotlin.stdlib)
    implementation(libs.androidx.annotations)
    implementation(project(":filament-android"))
}
