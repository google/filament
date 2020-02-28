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

import java.nio.file.Paths
import org.gradle.internal.os.OperatingSystem
import org.gradle.api.*
import org.gradle.api.logging.*
import org.gradle.api.tasks.*
import org.gradle.api.tasks.incremental.*

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
class MaterialCompiler extends TaskWithBinary {
    @SuppressWarnings("GroovyUnusedDeclaration")
    @InputDirectory
    File inputDir

    @OutputDirectory
    File outputDir

    MaterialCompiler() {
        super("matc")
    }

    @SuppressWarnings("GroovyUnusedDeclaration")
    @TaskAction
    void execute(IncrementalTaskInputs inputs) {
        if (!inputs.incremental) {
            project.delete(project.fileTree(outputDir).matching { include '*.filamat' })
        }

        inputs.outOfDate { InputFileDetails outOfDate ->
            if (outOfDate.file.directory) return
            def file = outOfDate.file

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

        inputs.removed { InputFileDetails removed ->
            getOutputFile(removed.file).delete()
        }
    }

    File getOutputFile(final File file) {
        return new File(outputDir, file.name[0..file.name.lastIndexOf('.')] + 'filamat')
    }
}

// Custom task to process IBLs using cmgen
// This task handles incremental builds
class IblGenerator extends TaskWithBinary {
    String cmgenArgs = null

    @SuppressWarnings("GroovyUnusedDeclaration")
    @InputFile
    File inputFile

    @OutputDirectory
    File outputDir

    IblGenerator() {
        super("cmgen")
    }

    @SuppressWarnings("GroovyUnusedDeclaration")
    @TaskAction
    void execute(IncrementalTaskInputs inputs) {
        if (!inputs.incremental) {
            project.delete(project.fileTree(outputDir).matching { include '*' })
        }

        inputs.outOfDate { InputFileDetails outOfDate ->
            def file = outOfDate.file

            def out = new LogOutputStream(logger, LogLevel.LIFECYCLE)
            def err = new LogOutputStream(logger, LogLevel.ERROR)

            def header = ("Generating IBL " + file + "\n").getBytes()
            out.write(header)
            out.flush()

            if (!getBinary().exists()) {
                throw new GradleException("Could not find ${getBinary()}." +
                        " Ensure Filament has been built/installed before building this app.")
            }

            project.exec {
                standardOutput out
                if (!cmgenArgs) {
                    cmgenArgs = '-q -x ' + outputDir +
                            ' --format=rgb32f --extract-blur=0.08 --extract=' + outputDir.absolutePath
                }
                cmgenArgs = cmgenArgs + " " + file
                errorOutput err
                executable "${getBinary()}"
                args(cmgenArgs.split())
            }
        }

        inputs.removed { InputFileDetails removed ->
            getOutputFile(removed.file).delete()
        }
    }

    File getOutputFile(final File file) {
        return new File(outputDir, file.name[0..file.name.lastIndexOf('.') - 1])
    }
}

// Custom task to compile mesh files using filamesh
// This task handles incremental builds
class MeshCompiler extends TaskWithBinary {
    @SuppressWarnings("GroovyUnusedDeclaration")
    @InputFile
    File inputFile

    @OutputDirectory
    File outputDir

    MeshCompiler() {
        super("filamesh")
    }

    @SuppressWarnings("GroovyUnusedDeclaration")
    @TaskAction
    void execute(IncrementalTaskInputs inputs) {
        if (!inputs.incremental) {
            project.delete(project.fileTree(outputDir).matching { include '*.filamesh' })
        }

        inputs.outOfDate { InputFileDetails outOfDate ->
            def file = outOfDate.file

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

        inputs.removed { InputFileDetails removed ->
            getOutputFile(removed.file).delete()
        }
    }

    File getOutputFile(final File file) {
        return new File(outputDir, file.name[0..file.name.lastIndexOf('.')] + 'filamesh')
    }
}

class FilamentToolsPluginExtension {
    File materialInputDir
    File materialOutputDir

    String cmgenArgs
    File iblInputFile
    File iblOutputDir

    File meshInputFile
    File meshOutputDir
}

class FilamentToolsPlugin implements Plugin<Project> {
    void apply(Project project) {
        def extension = project.extensions.create('filamentTools', FilamentToolsPluginExtension)

        project.ext.filamentToolsPath = project.file("../../../out/release/filament")
        if (project.hasProperty("filament_tools_dir")) {
            project.ext.filamentToolsPath = project.file(project.property("filament_tools_dir"))
        }

        project.tasks.register("filamentCompileMaterials", MaterialCompiler) {
            enabled = extension.materialInputDir != null && extension.materialOutputDir != null
            inputDir = extension.materialInputDir
            outputDir = extension.materialOutputDir
        }

        project.preBuild.dependsOn "filamentCompileMaterials"

        project.tasks.register("filamentGenerateIbl", IblGenerator) {
            enabled = extension.iblInputFile != null && extension.iblOutputDir != null
            cmgenArgs = extension.cmgenArgs
            inputFile = extension.iblInputFile
            outputDir = extension.iblOutputDir
        }

        project.preBuild.dependsOn "filamentGenerateIbl"

        project.tasks.register("filamentCompileMesh", MeshCompiler) {
            enabled = extension.meshInputFile != null && extension.meshOutputDir != null
            inputFile = extension.meshInputFile
            outputDir = extension.meshOutputDir
        }

        project.preBuild.dependsOn "filamentCompileMesh"
    }
}
