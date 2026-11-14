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

import com.android.build.api.dsl.LibraryExtension
import java.io.File
import org.gradle.api.JavaVersion
import org.gradle.api.Plugin
import org.gradle.api.Project
import org.gradle.api.artifacts.VersionCatalogsExtension
import org.gradle.api.publish.PublishingExtension
import org.gradle.api.publish.maven.MavenPublication

class FilamentLibraryPlugin : Plugin<Project> {
    override fun apply(project: Project) {
        project.pluginManager.apply("com.android.library")
        project.pluginManager.apply("com.google.android.filament.publish")

        project.repositories.mavenCentral()
        project.repositories.google()

        val libs = project.extensions.getByType(VersionCatalogsExtension::class.java).named("libs")
        val providers = project.providers

        val distDirProp = providers.gradleProperty("com.google.android.filament.dist-dir").get()
        val distFile = File(distDirProp).let {
            if (it.isAbsolute) it else File(project.rootDir, distDirProp)
        }
        val filamentPath = distFile.canonicalFile.absolutePath.replace(File.separator, "/")

        val excludeVulkan = providers.gradleProperty("com.google.android.filament.exclude-vulkan").isPresent
        val includeWebGPU = providers.gradleProperty("com.google.android.filament.include-webgpu").isPresent
        val fgviewer = providers.gradleProperty("com.google.android.filament.fgviewer").isPresent
        val matdbg = providers.gradleProperty("com.google.android.filament.matdbg").isPresent
        val matnopt = providers.gradleProperty("com.google.android.filament.matnopt").isPresent
        val mutexdebug = providers.gradleProperty("com.google.android.filament.mutexdebug").isPresent

        val configuredAbis = providers.gradleProperty("com.google.android.filament.abis").get().split(",")
        val abis = if (configuredAbis.contains("all")) {
            listOf("arm64-v8a", "armeabi-v7a", "x86_64", "x86")
        } else {
            configuredAbis
        }

        val cmakeArgs = listOf(
            "--no-warn-unused-cli",
            "-DANDROID_WEAK_API_DEFS=ON",
            "-DANDROID_PIE=ON",
            "-DANDROID_PLATFORM=21",
            "-DANDROID_STL=c++_static",
            "-DFILAMENT_DIST_DIR=$filamentPath",
            "-DFILAMENT_SUPPORTS_VULKAN=${if (excludeVulkan) "OFF" else "ON"}",
            "-DFILAMENT_SUPPORTS_WEBGPU=${if (includeWebGPU) "ON" else "OFF"}",
            "-DFILAMENT_ENABLE_FGVIEWER=${if (fgviewer) "ON" else "OFF"}",
            "-DFILAMENT_ENABLE_MATDBG=${if (matdbg) "ON" else "OFF"}",
            "-DFILAMENT_DISABLE_MATOPT=${if (matnopt) "ON" else "OFF"}",
            "-DFILAMENT_DEBUG_MUTEX=${if (mutexdebug) "ON" else "OFF"}"
        )

        val cppFlags = listOf(
            "-std=c++17",
            "-fno-stack-protector",
            "-fno-unwind-tables",
            "-fno-asynchronous-unwind-tables",
            "-fno-rtti",
            "-ffast-math",
            "-fno-finite-math-only",
            "-ffp-contract=fast",
            "-fvisibility-inlines-hidden",
            "-fvisibility=hidden",
            "-fomit-frame-pointer",
            "-ffunction-sections",
            "-fdata-sections",
            "-no-canonical-prefixes",
            "-Wformat",
            "-Werror=format-security",
            "-Wno-unused-command-line-argument",
            "-Wl,--gc-sections",
            "-Wl,-Bsymbolic-functions",
            "-Wl,--hash-style=both"
        )

        project.extensions.configure(LibraryExtension::class.java) { android ->
            android.buildToolsVersion = libs.findVersion("buildTools").get().requiredVersion
            android.compileSdk = libs.findVersion("compileSdk").get().requiredVersion.toInt()
            android.ndkVersion = libs.findVersion("ndk").get().requiredVersion

            android.defaultConfig {
                minSdk = libs.findVersion("minSdk").get().requiredVersion.toInt()

                externalNativeBuild {
                    cmake {
                        arguments.addAll(cmakeArgs)
                        this.cppFlags.addAll(cppFlags)
                    }
                }

                ndk {
                    abiFilters.addAll(abis)
                }

                consumerProguardFiles("../proguard-rules.pro")
            }

            android.externalNativeBuild {
                cmake {
                    version = libs.findVersion("cmake").get().requiredVersion
                    path = project.file("CMakeLists.txt")
                }
            }

            android.compileOptions {
                val jdkVersion = JavaVersion.toVersion(libs.findVersion("jdk").get().requiredVersion)
                sourceCompatibility = jdkVersion
                targetCompatibility = jdkVersion
            }

            android.publishing {
                singleVariant("release") {
                    withSourcesJar()
                    withJavadocJar()
                }
            }
        }

        val artifactId = (project.findProperty("POM_ARTIFACT_ID") as String?)
            ?: (project.findProperty("POM_ARTIFACT_ID_FULL") as String?)
            ?: project.name

        project.afterEvaluate {
            project.extensions.configure(PublishingExtension::class.java) { publishing ->
                publishing.publications.register("release", MavenPublication::class.java) { pub ->
                    pub.artifactId = artifactId
                    pub.from(project.components.getByName("release"))
                }
            }
        }
    }
}
