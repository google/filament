plugins {
    id("com.android.library")
    `maven-publish`
}

kotlin {
    jvmToolchain(libs.versions.jdk.get().toInt())
}

android {
    namespace = "com.google.android.filament.utils"

    sourceSets {
        getByName("main") {
            kotlin.directories.add("src/main/java")
        }
    }

    packaging {
        // No need to package up the following shared libs, which arise as a side effect of our
        // externalNativeBuild dependencies. When clients pick and choose from project-level gradle
        // dependencies, these shared libs already get pulled in, so we need to avoid the error:
        // "More than one file was found with OS independent path ..."
        jniLibs {
            excludes += listOf("lib/*/libfilament-jni.so", "lib/*/libgltfio-jni.so")
        }
    }

    publishing {
        singleVariant("release") {
            withSourcesJar()
            withJavadocJar()
        }
    }
}

dependencies {
    implementation(libs.kotlin.stdlib)
    implementation(libs.androidx.annotations)

    implementation(libs.kotlinx.coroutines.core)
    implementation(libs.kotlinx.coroutines.android)

    api(project(":filament-android"))
    api(project(":gltfio-android"))
}

apply(from = rootProject.file("gradle/gradle-mvn-push.gradle.kts"))

afterEvaluate {
    publishing {
        publications {
            register<MavenPublication>("release") {
                artifactId = project.property("POM_ARTIFACT_ID") as String
                from(components["release"])
            }
        }
    }
}
