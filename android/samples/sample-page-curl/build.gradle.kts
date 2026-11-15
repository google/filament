plugins {
    id("com.google.android.filament.sample")
    id("com.google.android.filament-tools")
}

filament {
    iblInputFile.set(project.layout.projectDirectory.file("../../../third_party/environments/studio_small_02_2k.hdr"))
    materialInputDir.set(project.layout.projectDirectory.dir("src/main/materials"))
}

android {
    namespace = "com.google.android.filament.pagecurl"
    defaultConfig {
        applicationId = "com.google.android.filament.pagecurl"
    }
}

dependencies {
    implementation(project(":filament-android"))
    implementation(libs.androidx.annotations)
}
