/*
 * Copyright (C) 2026 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.google.android.filament.buildlogic

import com.android.build.api.dsl.ApplicationExtension
import org.gradle.api.JavaVersion
import org.gradle.api.Plugin
import org.gradle.api.Project
import org.gradle.api.artifacts.VersionCatalogsExtension
import org.jetbrains.kotlin.gradle.dsl.KotlinBaseExtension

class FilamentSamplePlugin : Plugin<Project> {
    override fun apply(project: Project) {
        project.pluginManager.apply("com.android.application")

        project.repositories.mavenCentral()
        project.repositories.google()

        val libs = project.extensions.getByType(VersionCatalogsExtension::class.java).named("libs")
        val jdkVersionString = libs.findVersion("jdk").get().requiredVersion

        project.extensions.configure(KotlinBaseExtension::class.java) { kotlin ->
            kotlin.jvmToolchain(jdkVersionString.toInt())
        }

        project.extensions.configure(ApplicationExtension::class.java) { android ->
            android.compileSdk = libs.findVersion("compileSdk").get().requiredVersion.toInt()

            android.defaultConfig {
                minSdk = libs.findVersion("minSdk").get().requiredVersion.toInt()
                targetSdk = libs.findVersion("targetSdk").get().requiredVersion.toInt()
            }

            android.dependenciesInfo {
                includeInApk = false
            }

            android.androidResources {
                noCompress += listOf("filamat", "ktx")
            }

            android.compileOptions {
                val jdkVersion = JavaVersion.toVersion(jdkVersionString)
                sourceCompatibility = jdkVersion
                targetCompatibility = jdkVersion
            }
        }
    }
}
