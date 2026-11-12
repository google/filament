plugins {
    id("com.android.library")
    `maven-publish`
}

android {
    namespace = "com.google.android.filament.gltfio"

    packaging {
        // No need to package up the following shared libs, which arise as a side effect of our
        // externalNativeBuild dependencies. When clients pick and choose from project-level gradle
        // dependencies, these shared libs already get pulled in, so we need to avoid the error:
        // "More than one file was found with OS independent path ..."
        jniLibs {
            excludes += listOf("lib/*/libfilament-jni.so")
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
    implementation(libs.androidx.annotations)

    api(project(":filament-android"))
}

apply(from = rootProject.file("gradle/gradle-mvn-push.gradle.kts"))

afterEvaluate {
    publishing {
        publications {
            register<MavenPublication>("release") {
                artifactId = project.property("POM_ARTIFACT_ID_FULL") as String
                from(components["release"])
            }
        }
    }
}
