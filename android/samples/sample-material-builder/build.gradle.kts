plugins {
    id("com.google.android.filament.sample")
    id("com.google.android.filament-tools")
}

filament {
    meshInputFile.set(project.layout.projectDirectory.file("../../../third_party/models/shader_ball/shader_ball.obj"))
    iblInputFile.set(project.layout.projectDirectory.file("../../../third_party/environments/flower_road_no_sun_2k.hdr"))
}

android {
    namespace = "com.google.android.filament.material_builder"
    defaultConfig {
        applicationId = "com.google.android.filament.material_builder"
    }
}

dependencies {
    implementation(libs.kotlin.stdlib)
    implementation(project(":filament-android"))
    implementation(project(":filamat-android"))
}
