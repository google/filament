plugins {
    id("com.google.android.filament.sample")
    id("com.google.android.filament-tools")
}

filament {
    materialInputDir.set(project.layout.projectDirectory.dir("src/main/materials"))
}

android {
    namespace = "com.google.android.filament.proceduraltexturequad"
    defaultConfig {
        applicationId = "com.google.android.filament.proceduraltexturequad"
    }
}

dependencies {
    implementation(libs.kotlin.stdlib)
    implementation(project(":filament-android"))
    implementation(project(":filament-utils-android"))
}
