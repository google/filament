plugins {
    `java-gradle-plugin`
    alias(libs.plugins.kotlin.jvm)
}

gradlePlugin {
    plugins {
        create("filament-publish") {
            id = "com.google.android.filament.publish"
            implementationClass = "com.google.android.filament.buildlogic.FilamentPublishPlugin"
        }
        create("filament-library") {
            id = "com.google.android.filament.library"
            implementationClass = "com.google.android.filament.buildlogic.FilamentLibraryPlugin"
        }
        create("filament-sample") {
            id = "com.google.android.filament.sample"
            implementationClass = "com.google.android.filament.buildlogic.FilamentSamplePlugin"
        }
    }
}

dependencies {
    implementation(libs.android.gradle.plugin)
    implementation(libs.kotlin.gradle.plugin)
}
