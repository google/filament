// This plugin accepts the following parameters:
//
// filament_tools_dir
//     Path to the Filament distribution/install directory for desktop.
//     This directory must contain bin/matc.
//
// filament_supports_vulkan
//     When set, support for Vulkan will be enabled
//
// Example:
//     ./gradlew -Pfilament_tools_dir=../../dist-release assembleDebug

import org.gradle.api.DefaultTask
import org.gradle.api.GradleException
import org.gradle.api.Plugin
import org.gradle.api.Project
import org.gradle.api.file.DirectoryProperty
import org.gradle.api.file.FileType
import org.gradle.api.file.RegularFileProperty
import org.gradle.api.logging.LogLevel
import org.gradle.api.logging.Logger
import org.gradle.api.tasks.InputDirectory
import org.gradle.api.tasks.InputFile
import org.gradle.api.tasks.OutputDirectory
import org.gradle.api.tasks.TaskAction
import org.gradle.api.tasks.incremental.InputFileDetails
import org.gradle.internal.os.OperatingSystem
import org.gradle.work.ChangeType
import org.gradle.work.Incremental
import org.gradle.work.InputChanges

import java.nio.file.Paths

class TaskWithBinary extends DefaultTask {
    private final String binaryName
    private File binaryPath = null

    TaskWithBinary(String name) {
        binaryName = name
    }

    String getBinaryName() {
        return binaryName
    }

    File getBinary() {
        if (binaryPath == null) {
            def tool = ["/bin/${binaryName}.exe", "/bin/${binaryName}"]
            def fullPath = tool.collect { path ->
                Paths.get(project.ext.filamentToolsPath.absolutePath, path).toFile()
            }

            binaryPath = OperatingSystem.current().isWindows() ? fullPath[0] : fullPath[1]
        }
        return binaryPath
    }
}

class LogOutputStream extends ByteArrayOutputStream {
    private final Logger logger
    private final LogLevel level

    LogOutputStream(Logger logger, LogLevel level) {
        this.logger = logger
        this.level = level
    }

    Logger getLogger() {
        return logger
    }

    LogLevel getLevel() {
        return level
    }

    @Override
    void flush() {
        logger.log(level, toString())
        reset()
    }
}

// Custom task to compile material files using matc
// This task handles incremental builds
abstract class MaterialCompiler extends TaskWithBinary {
    @Incremental
    @InputDirectory
    final DirectoryProperty inputDir = project.objects.directoryProperty()

    @OutputDirectory
    final DirectoryProperty outputDir = project.objects.directoryProperty()

    MaterialCompiler() {
        super("matc")
    }

    @TaskAction
    void execute(InputChanges inputs) {
        if (!inputs.incremental) {
            project.delete(project.fileTree(outputDir.asFile.get()).matching { include '*.filamat' })
        }

        inputs.getFileChanges(inputDir).each { InputFileDetails change ->
            if (change.fileType == FileType.DIRECTORY) return

            def file = change.file

            if (change.changeType == ChangeType.REMOVED) {
                getOutputFile(file).delete()
            } else {
                def out = new LogOutputStream(logger, LogLevel.LIFECYCLE)
                def err = new LogOutputStream(logger, LogLevel.ERROR)

                def header = ("Compiling material " + file + "\n").getBytes()
                out.write(header)
                out.flush()

                if (!getBinary().exists()) {
                    throw new GradleException("Could not find ${getBinary()}." +
                            " Ensure Filament has been built/installed before building this app.")
                }

                def matcArgs = []
                if (project.hasProperty("filament_supports_vulkan")) {
                    matcArgs += ['-a', 'vulkan']
                }
                matcArgs += ['-a', 'opengl', '-p', 'mobile', '-o', getOutputFile(file), file]

                project.exec {
                    standardOutput out
                    errorOutput err
                    executable "${getBinary()}"
                    args matcArgs
                }
            }
        }
    }

    File getOutputFile(final File file) {
        return outputDir.file(file.name[0..file.name.lastIndexOf('.')] + 'filamat').get().asFile
    }
}

// Custom task to process IBLs using cmgen
// This task handles incremental builds
class IblGenerator extends TaskWithBinary {
    String cmgenArgs = null

    @Incremental
    @InputFile
    final RegularFileProperty inputFile = project.objects.fileProperty()

    @OutputDirectory
    final DirectoryProperty outputDir = project.objects.directoryProperty()

    IblGenerator() {
        super("cmgen")
    }

    @TaskAction
    void execute(InputChanges inputs) {
        if (!inputs.incremental) {
            project.delete(project.fileTree(outputDir.asFile.get()).matching { include '*' })
        }

        inputs.getFileChanges(inputFile).each { InputFileDetails change ->
            if (change.fileType == FileType.DIRECTORY) return

            def file = change.file

            if (change.changeType == ChangeType.REMOVED) {
                getOutputFile(file).delete()
            } else {
                def out = new LogOutputStream(logger, LogLevel.LIFECYCLE)
                def err = new LogOutputStream(logger, LogLevel.ERROR)

                def header = ("Generating IBL " + file + "\n").getBytes()
                out.write(header)
                out.flush()

                if (!getBinary().exists()) {
                    throw new GradleException("Could not find ${getBinary()}." +
                            " Ensure Filament has been built/installed before building this app.")
                }

                def outputPath = outputDir.get().asFile
                project.exec {
                    standardOutput out
                    if (!cmgenArgs) {
                        cmgenArgs =
                                '-q -x ' + outputPath + ' --format=rgb32f ' +
                                '--extract-blur=0.08 --extract=' + outputPath.absolutePath
                    }
                    cmgenArgs = cmgenArgs + " " + file
                    errorOutput err
                    executable "${getBinary()}"
                    args(cmgenArgs.split())
                }
            }
        }
    }

    File getOutputFile(final File file) {
        return outputDir.file(file.name[0..file.name.lastIndexOf('.') - 1]).get().asFile
    }
}

// Custom task to compile mesh files using filamesh
// This task handles incremental builds
class MeshCompiler extends TaskWithBinary {
    @Incremental
    @InputFile
    final RegularFileProperty inputFile = project.objects.fileProperty()

    @OutputDirectory
    final DirectoryProperty outputDir = project.objects.directoryProperty()

    MeshCompiler() {
        super("filamesh")
    }

    @TaskAction
    void execute(InputChanges inputs) {
        if (!inputs.incremental) {
            project.delete(project.fileTree(outputDir.asFile.get()).matching { include '*.filamesh' })
        }

        inputs.getFileChanges(inputFile).each { InputFileDetails change ->
            if (change.fileType == FileType.DIRECTORY) return

            def file = change.file

            if (change.changeType == ChangeType.REMOVED) {
                getOutputFile(file).delete()
            } else {
                def out = new LogOutputStream(logger, LogLevel.LIFECYCLE)
                def err = new LogOutputStream(logger, LogLevel.ERROR)

                def header = ("Compiling mesh " + file + "\n").getBytes()
                out.write(header)
                out.flush()

                if (!getBinary().exists()) {
                    throw new GradleException("Could not find ${getBinary()}." +
                            " Ensure Filament has been built/installed before building this app.")
                }

                project.exec {
                    standardOutput out
                    errorOutput err
                    executable "${getBinary()}"
                    args(file, getOutputFile(file))
                }
            }
        }
    }

    File getOutputFile(final File file) {
        return outputDir.file(file.name[0..file.name.lastIndexOf('.')] + 'filamesh').get().asFile
    }
}

class FilamentToolsPluginExtension {
    DirectoryProperty materialInputDir
    DirectoryProperty materialOutputDir

    String cmgenArgs
    RegularFileProperty iblInputFile
    DirectoryProperty iblOutputDir

    RegularFileProperty meshInputFile
    DirectoryProperty meshOutputDir
}

class FilamentToolsPlugin implements Plugin<Project> {
    void apply(Project project) {
        def extension = project.extensions.create('filamentTools', FilamentToolsPluginExtension)
        extension.materialInputDir = project.objects.directoryProperty()
        extension.materialOutputDir = project.objects.directoryProperty()
        extension.iblInputFile = project.objects.fileProperty()
        extension.iblOutputDir = project.objects.directoryProperty()
        extension.meshInputFile = project.objects.fileProperty()
        extension.meshOutputDir = project.objects.directoryProperty()

        project.ext.filamentToolsPath = project.file("../../../out/release/filament")
        if (project.hasProperty("filament_tools_dir")) {
            project.ext.filamentToolsPath = project.file(project.property("filament_tools_dir"))
        }

        project.tasks.register("filamentCompileMaterials", MaterialCompiler) {
            enabled =
                    extension.materialInputDir.isPresent() &&
                    extension.materialOutputDir.isPresent()
            inputDir = extension.materialInputDir.getOrNull()
            outputDir = extension.materialOutputDir.getOrNull()
        }

        project.preBuild.dependsOn "filamentCompileMaterials"

        project.tasks.register("filamentGenerateIbl", IblGenerator) {
            enabled = extension.iblInputFile.isPresent() && extension.iblOutputDir.isPresent()
            cmgenArgs = extension.cmgenArgs
            inputFile = extension.iblInputFile.getOrNull()
            outputDir = extension.iblOutputDir.getOrNull()
        }

        project.preBuild.dependsOn "filamentGenerateIbl"

        project.tasks.register("filamentCompileMesh", MeshCompiler) {
            enabled = extension.meshInputFile.isPresent() && extension.meshOutputDir.isPresent()
            inputFile = extension.meshInputFile.getOrNull()
            outputDir = extension.meshOutputDir.getOrNull()
        }

        project.preBuild.dependsOn "filamentCompileMesh"
    }
}
