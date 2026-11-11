plugins {
    `java-gradle-plugin`
    alias(libs.plugins.kotlin.jvm)
    alias(libs.plugins.plugin.publish)
}

group = "com.google.android.filament"
version = "0.1.0"

gradlePlugin {
    website.set("https://github.com/google/filament/tree/main/android/gradle-plugin")
    vcsUrl.set("https://github.com/google/filament/tree/main/android/gradle-plugin")

    plugins {
        create("filament-tools") {
            id = "com.google.android.filament-tools"
            displayName = "Filament Tools Gradle Plugin"
            description = "A plugin that helps integrate Filament into Android projects"
            tags.addAll("android", "graphics", "rendering", "filament", "3d", "gltf", "native")
            implementationClass = "com.google.android.filament.gradle.FilamentPlugin"
        }
    }
}

repositories {
    mavenCentral()
    gradlePluginPortal()
}

dependencies {
    implementation(libs.osdetector.gradle.plugin)
}
