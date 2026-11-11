plugins {
    `java-gradle-plugin`
    kotlin("jvm") version "2.4.0"
    id("com.gradle.plugin-publish") version "2.1.0"
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
    implementation("com.google.gradle:osdetector-gradle-plugin:1.7.3")
}
