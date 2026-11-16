plugins {
    id("com.google.android.filament.sample")
    id("com.google.android.filament-tools")
}

filament {
    materialInputDir.set(project.layout.projectDirectory.dir("src/main/materials"))
}

android {
    namespace = "com.google.android.filament.proceduraleffect"
    defaultConfig {
        applicationId = "com.google.android.filament.proceduraleffect"
    }
}

dependencies {
    implementation(libs.kotlin.stdlib)
    implementation(project(":filament-android"))
    implementation(project(":filament-utils-android"))
}
