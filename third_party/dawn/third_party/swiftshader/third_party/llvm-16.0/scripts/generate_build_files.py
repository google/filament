#!/usr/bin/env python3

# Copyright 2023 The SwiftShader Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import contextlib
import os
import sys
from string import Template

## Generates `CMakeLists.txt`, `Android.bp`, and `BUILD.gn`

CMAKE_TEMPLATE_PATH = "template_CMakeLists.txt"
DESTINATION_CMAKELISTS_PATH = "../CMakeLists.txt"

ANDROID_BP_TEMPLATE_PATH = "template_Android.bp"
DESTINATION_ANDROID_BP_PATH = "../Android.bp"

BUILD_GN_TEMPLATE_PATH = "template_BUILD.gn"
DESTINATION_BUILD_GN_PATH = "../BUILD.gn"

# This custom template class changes the delimiter from '$' to
# '%$%'.
# This is needed because CMake build files use '$' extensively.
class CustomTemplate(Template):
	delimiter = '%$%'

@contextlib.contextmanager
def pushd(new_dir):
    previous_dir = os.getcwd()
    os.chdir(new_dir)
    try:
        yield
    finally:
        os.chdir(previous_dir)

# Returns a list of all the .cpp and .c files in the CWD.
def list_all_potentially_compiled_files():
    all_files = []
    for root, sub_folders, files in os.walk("."):
        for file in files:
            file_path = os.path.join(root,file)
            extensions = os.path.splitext(file_path)
            if extensions[1] == ".cpp" or extensions[1] == ".c":
                assert(file_path[0] == '.')
                # Trim the leading '.' out of the path.
                all_files.append(file_path[1:])
    all_files.sort()
    return all_files

# Returns the subset of `files` that do no starts with `banned_prefixes`
def exclude_files_with_prefixes(files, banned_prefixes):
    result = []
    for f in files:
        file_start_with_banned_prefix = False
        for prefix in banned_prefixes:
            if f.startswith(prefix):
                file_start_with_banned_prefix = True
                break
        if not file_start_with_banned_prefix:
            result.append(f)
    return result

# Returns the subset of `files` that starts `needed_prefix`
def keep_files_with_prefix(files, needed_prefix):
    result = []
    for f in files:
        if f.startswith(needed_prefix):
            result.append(f)
    return result

# Get all the files
with pushd("../llvm"):
    all_files = list_all_potentially_compiled_files()

# Split the `all_files`` in several groups
# LLVM common files
prefixes_of_files_not_needed_by_llvm = [
    # The following is a list of files and directories that were
    # removed during the process of making llvm 16 build.
    "/lib/DebugInfo/PDB/DIA/",
    "/lib/ExecutionEngine/OProfileJIT/OProfileWrapper.cpp",
    "/lib/ExecutionEngine/IntelJITEvents/IntelJITEventListener.cpp",
    "/lib/ExecutionEngine/PerfJITEvents/PerfJITEventListener.cpp",
    "/lib/ExecutionEngine/OProfileJIT",
    "/lib/Frontend/OpenACC/",
    "/lib/ObjCopy/",
    "/lib/Support/BLAKE3/",
    "/lib/Target/",
    "/lib/Testing/",
    "/lib/ToolDrivers/llvm-lib/LibDriver.cpp",
    "/lib/ToolDrivers/llvm-dlltool/DlltoolDriver.cpp",
    "/lib/WindowsManifest/WindowsManifestMerger.cpp",
    # The following is a list of files and directories founds by running
    # build/strip_cmakelists.sh on a build that uses CMakeLists.txt.
    # The files removed were copied here.
    "/lib/Analysis/AliasAnalysisSummary.cpp",
    "/lib/Analysis/Analysis.cpp",
    "/lib/Analysis/DevelopmentModeInlineAdvisor.cpp",
    "/lib/Analysis/DomPrinter.cpp",
    "/lib/Analysis/Interval.cpp",
    "/lib/Analysis/IntervalPartition.cpp",
    "/lib/Analysis/MLInlineAdvisor.cpp",
    "/lib/Analysis/MemDepPrinter.cpp",
    "/lib/Analysis/ModelUnderTrainingRunner.cpp",
    "/lib/Analysis/NoInferenceModelRunner.cpp",
    "/lib/Analysis/RegionPrinter.cpp",
    "/lib/Analysis/TFLiteUtils.cpp",
    "/lib/Analysis/Trace.cpp",
    "/lib/Analysis/TrainingLogger.cpp",
    "/lib/BinaryFormat/AMDGPUMetadataVerifier.cpp",
    "/lib/BinaryFormat/DXContainer.cpp",
    "/lib/BinaryFormat/ELF.cpp",
    "/lib/BinaryFormat/Minidump.cpp",
    "/lib/BinaryFormat/MsgPackDocument.cpp",
    "/lib/BinaryFormat/MsgPackDocumentYAML.cpp",
    "/lib/BinaryFormat/MsgPackReader.cpp",
    "/lib/BinaryFormat/MsgPackWriter.cpp",
    "/lib/Bitcode/Reader/BitReader.cpp",
    "/lib/Bitcode/Reader/BitcodeAnalyzer.cpp",
    "/lib/Bitcode/Writer/BitWriter.cpp",
    "/lib/Bitcode/Writer/BitcodeWriterPass.cpp",
    "/lib/CodeGen/AsmPrinter/ErlangGCPrinter.cpp",
    "/lib/CodeGen/AsmPrinter/OcamlGCPrinter.cpp",
    "/lib/CodeGen/CodeGenPassBuilder.cpp",
    "/lib/CodeGen/CommandFlags.cpp",
    "/lib/CodeGen/MIRParser/MILexer.cpp",
    "/lib/CodeGen/MIRParser/MIParser.cpp",
    "/lib/CodeGen/MIRParser/MIRParser.cpp",
    "/lib/CodeGen/MIRYamlMapping.cpp",
    "/lib/CodeGen/MLRegallocPriorityAdvisor.cpp",
    "/lib/CodeGen/MachinePassManager.cpp",
    "/lib/CodeGen/MultiHazardRecognizer.cpp",
    "/lib/CodeGen/NonRelocatableStringpool.cpp",
    "/lib/CodeGen/ParallelCG.cpp",
    "/lib/CodeGen/RegAllocScore.cpp",
    "/lib/CodeGen/VLIWMachineScheduler.cpp",
    "/lib/DWARFLinker/DWARFLinker.cpp",
    "/lib/DWARFLinker/DWARFLinkerCompileUnit.cpp",
    "/lib/DWARFLinker/DWARFLinkerDeclContext.cpp",
    "/lib/DWARFLinker/DWARFStreamer.cpp",
    "/lib/DWARFLinkerParallel/DWARFLinker.cpp",
    "/lib/DWP/DWP.cpp",
    "/lib/DWP/DWPError.cpp",
    "/lib/DebugInfo/CodeView/AppendingTypeTableBuilder.cpp",
    "/lib/DebugInfo/CodeView/CVSymbolVisitor.cpp",
    "/lib/DebugInfo/CodeView/DebugChecksumsSubsection.cpp",
    "/lib/DebugInfo/CodeView/DebugCrossExSubsection.cpp",
    "/lib/DebugInfo/CodeView/DebugCrossImpSubsection.cpp",
    "/lib/DebugInfo/CodeView/DebugFrameDataSubsection.cpp",
    "/lib/DebugInfo/CodeView/DebugInlineeLinesSubsection.cpp",
    "/lib/DebugInfo/CodeView/DebugLinesSubsection.cpp",
    "/lib/DebugInfo/CodeView/DebugStringTableSubsection.cpp",
    "/lib/DebugInfo/CodeView/DebugSubsection.cpp",
    "/lib/DebugInfo/CodeView/DebugSubsectionRecord.cpp",
    "/lib/DebugInfo/CodeView/DebugSubsectionVisitor.cpp",
    "/lib/DebugInfo/CodeView/DebugSymbolRVASubsection.cpp",
    "/lib/DebugInfo/CodeView/DebugSymbolsSubsection.cpp",
    "/lib/DebugInfo/CodeView/Formatters.cpp",
    "/lib/DebugInfo/CodeView/LazyRandomTypeCollection.cpp",
    "/lib/DebugInfo/CodeView/MergingTypeTableBuilder.cpp",
    "/lib/DebugInfo/CodeView/StringsAndChecksums.cpp",
    "/lib/DebugInfo/CodeView/SymbolDumper.cpp",
    "/lib/DebugInfo/CodeView/SymbolRecordHelpers.cpp",
    "/lib/DebugInfo/CodeView/SymbolSerializer.cpp",
    "/lib/DebugInfo/CodeView/TypeDumpVisitor.cpp",
    "/lib/DebugInfo/CodeView/TypeRecordHelpers.cpp",
    "/lib/DebugInfo/CodeView/TypeStreamMerger.cpp",
    "/lib/DebugInfo/DWARF/DWARFLocationExpression.cpp",
    "/lib/DebugInfo/GSYM/",
    "/lib/DebugInfo/LogicalView/",
    "/lib/DebugInfo/MSF/",
    "/lib/DebugInfo/PDB/",
    "/lib/DebugInfo/Symbolize/",
    "/lib/Debuginfod/",
    "/lib/ExecutionEngine/ExecutionEngine.cpp",
    "/lib/ExecutionEngine/ExecutionEngineBindings.cpp",
    "/lib/ExecutionEngine/GDBRegistrationListener.cpp",
    "/lib/ExecutionEngine/IntelJITEvents/jitprofiling.c",
    "/lib/ExecutionEngine/Interpreter/Execution.cpp",
    "/lib/ExecutionEngine/Interpreter/ExternalFunctions.cpp",
    "/lib/ExecutionEngine/Interpreter/Interpreter.cpp",
    "/lib/ExecutionEngine/MCJIT/MCJIT.cpp",
    "/lib/ExecutionEngine/Orc/COFFPlatform.cpp",
    "/lib/ExecutionEngine/Orc/COFFVCRuntimeSupport.cpp",
    "/lib/ExecutionEngine/Orc/CompileOnDemandLayer.cpp",
    "/lib/ExecutionEngine/Orc/DebugObjectManagerPlugin.cpp",
    "/lib/ExecutionEngine/Orc/DebuggerSupportPlugin.cpp",
    "/lib/ExecutionEngine/Orc/EPCDebugObjectRegistrar.cpp",
    "/lib/ExecutionEngine/Orc/EPCDynamicLibrarySearchGenerator.cpp",
    "/lib/ExecutionEngine/Orc/EPCEHFrameRegistrar.cpp",
    "/lib/ExecutionEngine/Orc/EPCGenericDylibManager.cpp",
    "/lib/ExecutionEngine/Orc/EPCGenericJITLinkMemoryManager.cpp",
    "/lib/ExecutionEngine/Orc/EPCGenericRTDyldMemoryManager.cpp",
    "/lib/ExecutionEngine/Orc/EPCIndirectionUtils.cpp",
    "/lib/ExecutionEngine/Orc/IRTransformLayer.cpp",
    "/lib/ExecutionEngine/Orc/IndirectionUtils.cpp",
    "/lib/ExecutionEngine/Orc/LLJIT.cpp",
    "/lib/ExecutionEngine/Orc/LazyReexports.cpp",
    "/lib/ExecutionEngine/Orc/LookupAndRecordAddrs.cpp",
    "/lib/ExecutionEngine/Orc/MapperJITLinkMemoryManager.cpp",
    "/lib/ExecutionEngine/Orc/MemoryMapper.cpp",
    "/lib/ExecutionEngine/Orc/ObjectTransformLayer.cpp",
    "/lib/ExecutionEngine/Orc/OrcABISupport.cpp",
    "/lib/ExecutionEngine/Orc/OrcV2CBindings.cpp",
    "/lib/ExecutionEngine/Orc/Shared/OrcRTBridge.cpp",
    "/lib/ExecutionEngine/Orc/Shared/SimpleRemoteEPCUtils.cpp",
    "/lib/ExecutionEngine/Orc/SimpleRemoteEPC.cpp",
    "/lib/ExecutionEngine/Orc/SpeculateAnalyses.cpp",
    "/lib/ExecutionEngine/Orc/Speculation.cpp",
    "/lib/ExecutionEngine/Orc/TargetProcess/ExecutorSharedMemoryMapperService.cpp",
    "/lib/ExecutionEngine/Orc/TargetProcess/JITLoaderGDB.cpp",
    "/lib/ExecutionEngine/Orc/TargetProcess/OrcRTBootstrap.cpp",
    "/lib/ExecutionEngine/Orc/TargetProcess/SimpleExecutorDylibManager.cpp",
    "/lib/ExecutionEngine/Orc/TargetProcess/SimpleExecutorMemoryManager.cpp",
    "/lib/ExecutionEngine/Orc/TargetProcess/SimpleRemoteEPCServer.cpp",
    "/lib/ExecutionEngine/RuntimeDyld/RuntimeDyldChecker.cpp",
    "/lib/ExecutionEngine/TargetSelect.cpp",
    "/lib/Extensions/Extensions.cpp",
    "/lib/FileCheck/FileCheck.cpp",
    "/lib/Frontend/HLSL/HLSLResource.cpp",
    "/lib/Frontend/OpenMP/OMP.cpp",
    "/lib/Frontend/OpenMP/OMPContext.cpp",
    "/lib/FuzzMutate/",
    "/lib/IR/Core.cpp",
    "/lib/IR/ReplaceConstant.cpp",
    "/lib/IR/StructuralHash.cpp",
    "/lib/IR/TypedPointerType.cpp",
    "/lib/IR/VectorBuilder.cpp",
    "/lib/InterfaceStub/",
    "/lib/LTO/",
    "/lib/LineEditor/LineEditor.cpp",
    "/lib/Linker/LinkModules.cpp",
    "/lib/MC/ConstantPools.cpp",
    "/lib/MC/MCAsmInfoGOFF.cpp",
    "/lib/MC/MCAsmInfoWasm.cpp",
    "/lib/MC/MCAsmInfoXCOFF.cpp",
    "/lib/MC/MCDisassembler/Disassembler.cpp",
    "/lib/MC/MCDisassembler/MCDisassembler.cpp",
    "/lib/MC/MCDisassembler/MCExternalSymbolizer.cpp",
    "/lib/MC/MCDisassembler/MCSymbolizer.cpp",
    "/lib/MC/MCInstrInfo.cpp",
    "/lib/MC/MCLabel.cpp",
    "/lib/MC/MCParser/COFFMasmParser.cpp",
    "/lib/MC/MCParser/MasmParser.cpp",
    "/lib/MC/MCTargetOptionsCommandFlags.cpp",
    "/lib/MC/MCWasmObjectTargetWriter.cpp",
    "/lib/MC/MCXCOFFObjectTargetWriter.cpp",
    "/lib/MCA/",
    "/lib/Object/ArchiveWriter.cpp",
    "/lib/Object/BuildID.cpp",
    "/lib/Object/COFFImportFile.cpp",
    "/lib/Object/COFFModuleDefinition.cpp",
    "/lib/Object/DXContainer.cpp",
    "/lib/Object/FaultMapParser.cpp",
    "/lib/Object/MachOUniversalWriter.cpp",
    "/lib/Object/Object.cpp",
    "/lib/Object/SymbolSize.cpp",
    "/lib/Object/WindowsMachineFlag.cpp",
    "/lib/ObjectYAML/",
    "/lib/Passes/PassBuilderBindings.cpp",
    "/lib/Passes/PassPlugin.cpp",
    "/lib/Passes/StandardInstrumentations.cpp",
    "/lib/ProfileData/Coverage/CoverageMapping.cpp",
    "/lib/ProfileData/Coverage/CoverageMappingReader.cpp",
    "/lib/ProfileData/Coverage/CoverageMappingWriter.cpp",
    "/lib/ProfileData/GCOV.cpp",
    "/lib/ProfileData/InstrProfWriter.cpp",
    "/lib/ProfileData/RawMemProfReader.cpp",
    "/lib/ProfileData/SampleProfWriter.cpp",
    "/lib/Remarks/Remark.cpp",
    "/lib/Remarks/RemarkLinker.cpp",
    "/lib/Support/AMDGPUMetadata.cpp",
    "/lib/Support/APFixedPoint.cpp",
    "/lib/Support/ARMWinEH.cpp",
    "/lib/Support/AddressRanges.cpp",
    "/lib/Support/Allocator.cpp",
    "/lib/Support/Atomic.cpp",
    "/lib/Support/AutoConvert.cpp",
    "/lib/Support/Base64.cpp",
    "/lib/Support/BuryPointer.cpp",
    "/lib/Support/COM.cpp",
    "/lib/Support/CSKYAttributeParser.cpp",
    "/lib/Support/CSKYAttributes.cpp",
    "/lib/Support/CachePruning.cpp",
    "/lib/Support/Caching.cpp",
    "/lib/Support/DAGDeltaAlgorithm.cpp",
    "/lib/Support/DeltaAlgorithm.cpp",
    "/lib/Support/FileCollector.cpp",
    "/lib/Support/FileOutputBuffer.cpp",
    "/lib/Support/FileUtilities.cpp",
    "/lib/Support/InitLLVM.cpp",
    "/lib/Support/LockFileManager.cpp",
    "/lib/Support/MSP430AttributeParser.cpp",
    "/lib/Support/MSP430Attributes.cpp",
    "/lib/Support/Parallel.cpp",
    "/lib/Support/PluginLoader.cpp",
    "/lib/Support/RWMutex.cpp",
    "/lib/Support/SHA256.cpp",
    "/lib/Support/SystemUtils.cpp",
    "/lib/Support/TarWriter.cpp",
    "/lib/Support/ThreadPool.cpp",
    "/lib/Support/UnicodeNameToCodepoint.cpp",
    "/lib/Support/UnicodeNameToCodepointGenerated.cpp",
    "/lib/Support/Watchdog.cpp",
    "/lib/Support/Z3Solver.cpp",
    "/lib/Support/raw_os_ostream.cpp",
    "/lib/TableGen/",
    "/lib/TargetParser/CSKYTargetParser.cpp",
    "/lib/TargetParser/LoongArchTargetParser.cpp",
    "/lib/TargetParser/RISCVTargetParser.cpp",
    "/lib/TargetParser/TargetParser.cpp",
    "/lib/TargetParser/X86TargetParser.cpp",
    "/lib/TextAPI/Symbol.cpp",
    "/lib/Transforms/Hello/Hello.cpp",
    "/lib/Transforms/IPO/BarrierNoopPass.cpp",
    "/lib/Transforms/IPO/ExtractGV.cpp",
    "/lib/Transforms/IPO/IPO.cpp",
    "/lib/Transforms/IPO/InlineSimple.cpp",
    "/lib/Transforms/IPO/PassManagerBuilder.cpp",
    "/lib/Transforms/IPO/ThinLTOBitcodeWriter.cpp",
    "/lib/Transforms/Scalar/PlaceSafepoints.cpp",
    "/lib/Transforms/Scalar/Scalar.cpp",
    "/lib/Transforms/Utils/AMDGPUEmitPrintf.cpp",
    "/lib/Transforms/Utils/LowerMemIntrinsics.cpp",
    "/lib/Transforms/Utils/SanitizerStats.cpp",
    "/lib/Transforms/Utils/SplitModule.cpp",
    "/lib/Transforms/Utils/Utils.cpp",
    "/lib/Transforms/Vectorize/VPlanSLP.cpp",
    "/lib/Transforms/Vectorize/Vectorize.cpp",
    "/lib/WindowsDriver/MSVCPaths.cpp",
    "/lib/XRay/",
    "/lib/Target/X86/Disassembler/X86Disassembler.cpp",
    "/lib/Target/X86/MCA/X86CustomBehaviour.cpp",
    "/lib/Target/Mips/Disassembler/MipsDisassembler.cpp",
    "/lib/Target/AArch64/Disassembler/AArch64Disassembler.cpp",
    "/lib/Target/ARM/Disassembler/ARMDisassembler.cpp",
    "/lib/Target/PowerPC/Disassembler/PPCDisassembler.cpp",
    "/lib/Target/RISCV/Disassembler/RISCVDisassembler.cpp",
]
files_to_add_back_for_llvm = [
    "/lib/Target/TargetLoweringObjectFile.cpp",
    "/lib/Target/TargetMachine.cpp",
    "/lib/Support/BLAKE3/blake3.c",
    "/lib/Support/BLAKE3/blake3_dispatch.c",
    "/lib/Support/BLAKE3/blake3_portable.c"
]
files_llvm = exclude_files_with_prefixes(all_files, prefixes_of_files_not_needed_by_llvm)
files_llvm.extend(files_to_add_back_for_llvm)

files_llvm_debug = [
    "/lib/Analysis/RegionPrinter.cpp",
    "/lib/MC/MCDisassembler/MCDisassembler.cpp",
]

files_to_add_back_for_llvm_arm = [
    "/lib/CodeGen/MultiHazardRecognizer.cpp",
    "/lib/MC/ConstantPools.cpp",
    "/lib/MC/MCInstrInfo.cpp",
    "/lib/Transforms/IPO/BarrierNoopPass.cpp",
]

files_to_add_back_for_llvm_loongarch = [
    "/lib/TargetParser/LoongArchTargetParser.cpp",
    "/lib/Transforms/IPO/BarrierNoopPass.cpp",
]

files_to_add_back_for_llvm_riscv = [
    "/lib/TargetParser/RISCVTargetParser.cpp",
    "/lib/Transforms/IPO/BarrierNoopPass.cpp",
]

# Architecture specific files
files_x86 = keep_files_with_prefix(all_files, "/lib/Target/X86/")
files_Mips = keep_files_with_prefix(all_files, "/lib/Target/Mips/")
files_AArch64 = keep_files_with_prefix(all_files, "/lib/Target/AArch64/")
files_AArch64.extend(files_to_add_back_for_llvm_arm)
files_AArch64.sort()
files_ARM = keep_files_with_prefix(all_files, "/lib/Target/ARM/")
files_ARM.extend(files_to_add_back_for_llvm_arm)
files_ARM.sort()
files_LoongArch = keep_files_with_prefix(all_files, "/lib/Target/LoongArch/")
files_LoongArch.extend(files_to_add_back_for_llvm_loongarch)
files_LoongArch.sort()
files_PowerPC = keep_files_with_prefix(all_files, "/lib/Target/PowerPC/")
files_RISCV = keep_files_with_prefix(all_files, "/lib/Target/RISCV/")
files_RISCV.extend(files_to_add_back_for_llvm_riscv)
files_RISCV.sort()

generated_file_comment = "File generated by " + sys.argv[0]

# Generate CMakeLists.txt
def format_file_list_for_cmake(files):
    return '\n'.join(["        ${LLVM_DIR}" + s for s in files])
cmake_template_data = {
    'generated_file_comment' : "# " + generated_file_comment,
    'files_llvm' : '\n'.join(["    ${LLVM_DIR}" + s for s in files_llvm]),
    'files_x86' : format_file_list_for_cmake(files_x86),
    'files_LoongArch' : format_file_list_for_cmake(files_LoongArch),
    'files_Mips' : format_file_list_for_cmake(files_Mips),
    'files_AArch64' : format_file_list_for_cmake(files_AArch64),
    'files_ARM' : format_file_list_for_cmake(files_ARM),
    'files_PowerPC' : format_file_list_for_cmake(files_PowerPC),
    'files_RISCV' : format_file_list_for_cmake(files_RISCV),
}
with open(CMAKE_TEMPLATE_PATH, 'r') as f:
    cmake_template = CustomTemplate(f.read())
    result = cmake_template.substitute(cmake_template_data)
    cmake_output = open(DESTINATION_CMAKELISTS_PATH, "w")
    cmake_output.write(result)

# Generate Android.bp
def format_file_list_for_android_bp(files, indent):
    spaces = '    ' * indent
    return '\n'.join([spaces + "\"llvm" + s + "\"," for s in files])
android_bp_template_data = {
    'generated_file_comment' : "// " + generated_file_comment,
    'files_llvm' : format_file_list_for_android_bp(files_llvm, indent=2),
    'files_llvm_debug': format_file_list_for_android_bp(files_llvm_debug, indent=2),
    'files_x86' : format_file_list_for_android_bp(files_x86, indent=4),
    'files_AArch64' : format_file_list_for_android_bp(files_AArch64, indent=4),
    'files_ARM' : format_file_list_for_android_bp(files_ARM, indent=4),
    'files_RISCV' : format_file_list_for_android_bp(files_RISCV, indent=4),
}
with open(ANDROID_BP_TEMPLATE_PATH, 'r') as f:
    android_bp_template = CustomTemplate(f.read())
    result = android_bp_template.substitute(android_bp_template_data)
    cmake_output = open(DESTINATION_ANDROID_BP_PATH, "w")
    cmake_output.write(result)

# Generate BUILD.gn

# Remove 3 files that are special cases.
# They are dealt with in the template file.
files_llvm_build_gn = exclude_files_with_prefixes(files_llvm, ['/lib/MC/MCWasmObjectTargetWriter.cpp', '/lib/MC/MCXCOFFObjectTargetWriter.cpp'])
files_ARM_build_gn = exclude_files_with_prefixes(files_ARM, ['/lib/Target/ARM/MCTargetDesc/ARMTargetStreamer.cpp'])

# GN doesn't allow for duplicate source file names, even if they are
# in different subdirectories. Because of this, `files_llvm_build_gn` is
# partitionned into multiple targets.
def get_filename(path):
        return path.split("/")[-1]
# Takes a list of paths and returns multiple lists of paths.
# Every list will contain a unique set of filenames.
def partition_paths(filepaths):
    partitions = []
    for path in filepaths:
        # Convert to lower case to support case-insensitive filesystem
        filename = get_filename(path).lower()
        inserted = False
        for partition in partitions:
            if not filename in partition:
                partition[filename] = path
                inserted = True
                break
        if not inserted:
            new_partition = {filename : path}
            partitions.append(new_partition)
    return [[p for f, p in partition.items()] for partition in partitions]
# Returns a string containing a GN source set containing the filepaths
def create_source_set(source_set_name, filepaths):
    s = 'swiftshader_llvm_source_set("%s") {\n' % source_set_name
    s += '  sources = [\n'
    for filepath in filepaths:
        s += '    "llvm%s",\n' % filepath
    s += '  ]\n'
    s += '}\n'
    return s
def source_set_name(i):
    return 'swiftshader_llvm_source_set_%i' % i

# Generate the GN source set code
files_llvm_partitions = partition_paths(files_llvm_build_gn)
source_set_code = ""
i = 0
for partition in files_llvm_partitions:
    source_set_code += create_source_set(source_set_name(i), partition)
    i = i+1

# Generate the GN deps code
deps_code = ''
i = 0
for partition in files_llvm_partitions:
    deps_code += '    ":%s",\n' % source_set_name(i)
    i = i+1

def format_file_list_for_build_gn(files):
    return '\n'.join(["    \"llvm" + s + "\"," for s in files])
build_gn_template_data = {
    'generated_file_comment' : "# " + generated_file_comment,
    'llvm_source_sets' : source_set_code,
    'llvm_deps' : deps_code,
    'files_x86' : format_file_list_for_build_gn(files_x86),
    'files_AArch64' : format_file_list_for_build_gn(files_AArch64),
    'files_ARM' : format_file_list_for_build_gn(files_ARM_build_gn),
    'files_LoongArch' : format_file_list_for_build_gn(files_LoongArch),
    'files_Mips' : format_file_list_for_build_gn(files_Mips),
    'files_PowerPC' : format_file_list_for_build_gn(files_PowerPC),
    'files_RISCV' : format_file_list_for_build_gn(files_RISCV),
    'files_llvm_debug': format_file_list_for_build_gn(files_llvm_debug),
}
with open(BUILD_GN_TEMPLATE_PATH, 'r') as f:
    build_gn_template = CustomTemplate(f.read())
    result = build_gn_template.substitute(build_gn_template_data)
    cmake_output = open(DESTINATION_BUILD_GN_PATH, "w")
    cmake_output.write(result)