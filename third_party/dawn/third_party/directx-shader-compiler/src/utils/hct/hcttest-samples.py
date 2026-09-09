# Copyright (C) Microsoft Corporation. All rights reserved.
# This file is distributed under the University of Illinois Open Source License. See LICENSE.TXT for details.
import argparse
import glob
import shutil
import os
import subprocess
import re

try:
    import xml.etree.cElementTree as ET
except ImportError:
    import xml.etree.ElementTree as ET


# Comment namespace to make working with ElementTree easier:
def ReadXmlString(text):
    global xmlheader
    text = text.replace("xmlns=", "xmlns_commented=")
    xmlheader = text[: text.find("\n") + 1]
    # TODO: read xml and return xml root
    return ET.fromstring(text)


def WriteXmlString(root):
    global xmlheader
    text = ET.tostring(root, encoding="utf-8")
    return xmlheader + text.replace("xmlns_commented=", "xmlns=")


def DeepCopyElement(e):
    cpy = ET.Element(e.tag, e.attrib)
    cpy.text = e.text
    cpy.tail = e.tail
    for child in e:
        cpy.append(DeepCopyElement(child))
    return cpy


sample_names = [
    "D3D1211On12",
    "D3D12Bundles",
    "D3D12DynamicIndexing",
    "D3D12ExecuteIndirect",
    "D3D12Fullscreen",
    #    "D3D12HelloWorld",
    "D3D12HelloWorld\\src\\HelloBundles",
    "D3D12HelloWorld\\src\\HelloConstBuffers",
    "D3D12HelloWorld\\src\\HelloFrameBuffering",
    "D3D12HelloWorld\\src\\HelloTexture",
    "D3D12HelloWorld\\src\\HelloTriangle",
    "D3D12HelloWorld\\src\\HelloWindow",
    "D3D12HeterogeneousMultiadapter",
    # WarpAssert in ReadPointerOperand: pInfoSrc->getName().startswith("dx.var.x").
    # Fix tested - incorrect rendered result produced.
    # Worked around in sample shaders for now by moving to non-static locals.
    "D3D12Multithreading",  # works and is visually impressive!
    "D3D12nBodyGravity",  # expected empty cbuffer culling due to static members causes this to fail!  FIXED!
    "D3D12PipelineStateCache",  # Requires workaround for static globals
    "D3D12PredicationQueries",
    "D3D12ReservedResources",
    "D3D12Residency",  # has problems on x86 due to mem limitations. The following seems to help:
    # change NumTextures to 1024 * 1, and add min with 1GB here:
    # UINT64 memoryToUse = UINT64 (float(min(memoryInfo.Budget, UINT64(1 << 30))) * 0.95f);
    # Still produces a bunch of these errors:
    # D3D12 ERROR: ID3D12CommandAllocator::Reset: A command allocator is being reset before previous executions associated with the allocator have completed. [ EXECUTION ERROR #552: COMMAND_ALLOCATOR_SYNC]
    # but at least it no longer crashes
    "D3D12SmallResources",
]


class Sample(object):
    def __init__(self, name):
        self.name = name
        self.preBuild = []
        self.postBuild = []


samples = dict([(name, Sample(name)) for name in sample_names])


def SetSampleActions():
    # Actions called with (name, args, dxil)
    # Actions for all projects:
    for name, sample in samples.items():
        sample.postBuild.append(ActionCopyD3DBins)
        sample.postBuild.append(ActionCopySDKLayers)
        sample.postBuild.append(ActionCopyWarp12)
        sample.postBuild.append(
            ActionCopyCompilerBins
        )  # Do this for all projects for now
    # TODO: limit ActionCopyCompilerBins action to ones that do run-time compilation.
    # # Runtime HLSL compilation:
    # for name in [   "D3D1211On12",
    #                 "D3D12HelloWorld\\src\\HelloTriangle",
    #                 "D3D12ExecuteIndirect",
    #             ]:
    #     samples[name].postBuild.append(ActionCopyCompilerBins)


def CopyFiles(source_path, dest_path, filenames, symbols=False):
    for filename in filenames:
        renamed = filename
        try:
            filename, renamed = filename.split(";")
            print("Copying %s from %s to %s" % (filename, source_path, dest_path))
            print(".. with new name: %s" % (renamed))
        except:
            print("Copying %s from %s to %s" % (filename, source_path, dest_path))
        try:
            shutil.copy2(
                os.path.join(source_path, filename), os.path.join(dest_path, renamed)
            )
        except:
            print(
                'Error copying "%s" from "%s" to "%s"'
                % (filename, source_path, dest_path)
            )
            # sys.excepthook(*sys.exc_info())
            continue
        if symbols and (filename.endswith(".exe") or filename.endswith(".dll")):
            symbol_filename = filename[:-4] + ".pdb"
            try:
                shutil.copy2(
                    os.path.join(source_path, symbol_filename),
                    os.path.join(dest_path, symbol_filename),
                )
            except:
                print(
                    'Error copying symbols "%s" from "%s" to "%s"'
                    % (symbol_filename, source_path, dest_path)
                )


def CopyBins(args, name, dxil, filenames, symbols=False):
    samples_path = args.samples
    config = dxil and "DxilDebug" or "Debug"
    CopyFiles(
        args.bins,
        os.path.join(PathToSampleSrc(samples_path, name), "bin", args.arch, config),
        filenames,
        symbols,
    )


def ActionCopyCompilerBins(args, name, dxil):
    if dxil:
        CopyBins(
            args,
            name,
            dxil,
            [
                "dxcompiler.dll",
                "dxil.dll",
                "d3dcompiler_dxc_bridge.dll;d3dcompiler_47.dll",  # Wrapper version that calls into dxcompiler.dll
            ],
            args.symbols,
        )


def ActionCopyD3DBins(args, name, dxil):
    if dxil:
        CopyBins(
            args,
            name,
            dxil,
            [
                "d3d12.dll",
            ],
            args.symbols,
        )


def ActionCopySDKLayers(args, name, dxil):
    CopyBins(
        args,
        name,
        dxil,
        [
            "D3D11_3SDKLayers.dll",
            "D3D12SDKLayers.dll",
            "DXGIDebug.dll",
        ],
        args.symbols,
    )


def ActionCopyWarp12(args, name, dxil):
    CopyBins(
        args,
        name,
        dxil,
        [
            "d3d12warp.dll",
        ],
        args.symbols,
    )


def MakeD3D12WarpCopy(bin_path):
    # Copy d3d10warp.dll to d3d12warp.dll
    shutil.copy2(
        os.path.join(bin_path, "d3d10warp.dll"), os.path.join(bin_path, "d3d12warp.dll")
    )


def PathSplitAll(p):
    s = filter(None, os.path.split(p))
    if len(s) > 1:
        return PathSplitAll(s[0]) + (s[1],)
    else:
        return (s[0],)


def GetBinPath(args, name):
    return os.path.join(args.bins, name)


def GetProjectBinFilePath(args, samples_path, sample_name, file_name):
    src = PathToSampleSrc(samples_path, sample_name)
    return os.path.join(src, "bin", args.arch, file_name)


def ListRuntimeCompilePaths(args):
    return [
        os.path.join(args.bins, name)
        for name in [
            "fxc.exe",
            "dxc.exe",
            "dxcompiler.dll",
            "dxil.dll",
            "d3dcompiler_47.dll",
            "d3d12.dll",
            "D3D11_3SDKLayers.dll",
            "D3D12SDKLayers.dll",
            "DXGIDebug.dll",
            "d3d12warp.dll",
        ]
    ]


def CheckEnvironment(args):
    if not args.bins:
        print("The -bins argument is needed to populate tool binaries.")
        exit(1)
    if not os.path.exists(args.bins):
        print("The -bins argument '" + args.bins + "' does not exist.")
        exit(1)
    for fn in ListRuntimeCompilePaths(args):
        if not os.path.exists(fn):
            print("Expected file '" + fn + "' not found.")
            exit(1)
    if os.path.getmtime(GetBinPath(args, "fxc.exe")) != os.path.getmtime(
        GetBinPath(args, "dxc.exe")
    ):
        print("fxc.exe should be a copy of dxc.exe.")
        print(
            "Please copy "
            + GetBinPath(args, "dxc.exe")
            + " "
            + GetBinPath(args, "fxc.exe")
        )
        exit(1)
    try:
        msbuild_version = subprocess.check_output(["msbuild", "-nologo", "-ver"])
        print("msbuild version: " + msbuild_version)
    except Exception as E:
        print("Unable to get the version from msbuild: " + str(E))
        print("This command should be run from a Developer Command Prompt")
        exit(1)


def SampleIsNested(name):
    return "\\src\\" in name


def PathToSampleSrc(basePath, sampleName):
    if SampleIsNested(sampleName):
        return os.path.join(basePath, "Samples", "Desktop", sampleName)
    return os.path.join(basePath, "Samples", "Desktop", sampleName, "src")


reConfig = r"(Debug|Release|DxilDebug|DxilRelease)\|(Win32|x64)"


def AddProjectConfigs(root, args):
    rxConfig = re.compile(reConfig)
    changed = False
    for e in root.findall(".//WindowsTargetPlatformVersion"):
        if e.text == "10.0.10240.0":
            e.text = "10.0.10586.0"
            changed = True
    # Override fxc path for Dxil configs:
    for config in ["DxilDebug", "DxilRelease"]:
        for arch in ["Win32", "x64"]:
            if not root.find(
                """./PropertyGroup[@Condition="'$(Configuration)|$(Platform)'=='%s|%s'"]/FXCToolPath"""
                % (config, arch)
            ):
                e = ET.Element(
                    "PropertyGroup",
                    {
                        "Condition": "'$(Configuration)|$(Platform)'=='%s|%s'"
                        % (config, arch)
                    },
                )
                e.text = "\n    "
                e.tail = "\n  "
                root.insert(0, e)
                e = ET.SubElement(e, "FXCToolPath")
                e.text = "$(DXC_BIN_PATH)"  # args.bins
                e.tail = "\n  "
    # Extend ProjectConfiguration for Win32 and Dxil configs
    ig = root.find('./ItemGroup[@Label="ProjectConfigurations"]') or []
    debug_config = release_config = None  # ProjectConfiguration
    configs = {}
    for e in ig:
        try:
            m = rxConfig.match(e.attrib["Include"])
            if m:
                key = m.groups()
                configs[key] = e
                if m.group(1) == "Debug":
                    debug_config = key, e
                elif m.group(1) == "Release":
                    release_config = key, e
        except:
            continue
    parents = root.findall(
        """.//*[@Condition="'$(Configuration)|$(Platform)'=='%s|%s'"]/.."""
        % ("Debug", "x64")
    )
    if not configs or not debug_config or not release_config:
        print("No ProjectConfigurations found")
        return False
    for arch in ["Win32", "x64"]:
        for config in ["Debug", "DxilDebug", "Release", "DxilRelease"]:
            if config in ("Debug", "DxilDebug"):
                t_config = "Debug", "x64"
            else:
                t_config = "Release", "x64"
            config_condition = "'$(Configuration)|$(Platform)'=='%s|%s'" % t_config
            if not configs.get((config, arch), None):
                changed = True
                if config in ("Debug", "DxilDebug"):
                    e = DeepCopyElement(debug_config[1])
                else:
                    e = DeepCopyElement(release_config[1])
                e.set("Include", "%s|%s" % (config, arch))
                e.find("./Configuration").text = config
                e.find("./Platform").text = arch
                ig.append(e)
                for parent in reversed(parents):
                    for n, e in reversed(list(enumerate(parent))):
                        try:
                            cond = e.attrib["Condition"]
                        except KeyError:
                            continue
                        if cond == config_condition:
                            e = DeepCopyElement(e)
                            # Override DisableOptimizations for DxilDebug since this flag is problematic right now
                            if e.tag == "ItemDefinitionGroup" and config == "DxilDebug":
                                FxCompile = e.find("./FxCompile") or ET.SubElement(
                                    e, "FxCompile"
                                )
                                DisableOptimizations = FxCompile.find(
                                    "./DisableOptimizations"
                                ) or ET.SubElement(FxCompile, "DisableOptimizations")
                                DisableOptimizations.text = "false"
                            e.attrib[
                                "Condition"
                            ] = "'$(Configuration)|$(Platform)'=='%s|%s'" % (
                                config,
                                arch,
                            )
                            parent.insert(n + 1, e)
    return changed


def AddSlnConfigs(sln_text):
    # sln: GlobalSection(SolutionConfigurationPlatforms)
    rxSlnConfig = re.compile(r"^\s+%s = \1\|\2$" % reConfig)
    # sln: GlobalSection(ProjectConfigurationPlatforms)
    rxActiveCfg = re.compile(r"^\s+\{[0-9A-Z-]+\}\.%s\.ActiveCfg = \1\|\2$" % reConfig)
    rxBuild = re.compile(r"^\s+\{[0-9A-Z-]+\}\.%s\.Build\.0 = \1\|\2$" % reConfig)
    sln_changed = []
    line_set = set(sln_text.splitlines())

    def add_line(lst, line):
        "Prevent duplicates from being added"
        if line not in line_set:
            lst.append(line)

    for line in sln_text.splitlines():
        if line == "VisualStudioVersion = 14.0.23107.0":
            sln_changed.append("VisualStudioVersion = 14.0.25123.0")
            continue
        m = rxSlnConfig.match(line)
        if not m:
            m = rxActiveCfg.match(line)
        if not m:
            m = rxBuild.match(line)
        if m:
            sln_changed.append(line)
            config = m.group(1)
            if config in ("Debug", "Release") and m.group(2) == "x64":
                add_line(sln_changed, line.replace("x64", "Win32"))
                add_line(sln_changed, line.replace(config, "Dxil" + config))
                add_line(
                    sln_changed,
                    line.replace(config, "Dxil" + config).replace("x64", "Win32"),
                )
            continue
        sln_changed.append(line)
    return "\n".join(sln_changed)


def PatchProjects(args):
    for name in sample_names + ["D3D12HelloWorld"]:
        sample_path = PathToSampleSrc(args.samples, name)
        print("Patching " + name + " in " + sample_path)
        for proj_path in glob.glob(os.path.join(sample_path, "*.vcxproj")):
            # Consider looking for msbuild and other tool paths in registry, etg:
            # reg query HKLM\Software\Microsoft\MSBuild\ToolsVersions\14.0
            with open(proj_path, "r") as proj_file:
                proj_text = proj_file.read()
            root = ReadXmlString(proj_text)
            if AddProjectConfigs(root, args):
                changed_text = WriteXmlString(root)
                print("Patching the Windows SDK version in " + proj_path)
                with open(proj_path, "w") as proj_file:
                    proj_file.write(changed_text)

        # Extend project configs in solution file
        for sln_path in glob.glob(os.path.join(sample_path, "*.sln")):
            with open(sln_path, "r") as sln_file:
                sln_text = sln_file.read()
            changed_text = AddSlnConfigs(sln_text)
            if changed_text != sln_text:
                print("Adding additional configurations to " + sln_path)
                with open(sln_path, "w") as sln_file:
                    sln_file.write(changed_text)


def BuildSample(samples_path, name, x86, dxil):
    sample_path = PathToSampleSrc(samples_path, name)
    if not SampleIsNested(name):
        print("Building " + name + " in " + sample_path)
        Platform = x86 and "Win32" or "x64"
        Configuration = dxil and "DxilDebug" or "Debug"
        subprocess.check_call(
            [
                "msbuild",
                "-nologo",
                "/p:Configuration=%s;Platform=%s" % (Configuration, Platform),
                "/t:Rebuild",
            ],
            cwd=sample_path,
        )


def BuildSamples(args, dxil):
    samples_path = args.samples
    rxSample = args.sample and re.compile(args.sample, re.I)
    buildHelloWorld = False
    os.environ["DXC_BIN_PATH"] = args.bins
    for sample_name in sample_names:
        if rxSample and not rxSample.match(sample_name):
            continue
        if SampleIsNested(sample_name):
            buildHelloWorld = True
            continue
        BuildSample(samples_path, sample_name, args.x86, dxil)
    if buildHelloWorld:
        # HelloWorld containts sub-samples that must be built from the solution file.
        BuildSample(samples_path, "D3D12HelloWorld", args.x86, dxil)


def PatchSample(args, name, dxil, afterBuild):
    if args.sample and not re.match(args.sample, name, re.I):
        return
    try:
        sample = samples[name]
    except:
        print("Error: selected sample missing from sample map '" + name + "'.")
        return
    if afterBuild:
        actions = sample.postBuild
    else:
        actions = sample.preBuild
    for action in actions:
        action(args, name, dxil)


def PatchSamplesAfterBuild(args, dxil):
    for sample_name in sample_names:
        PatchSample(args, sample_name, dxil, True)


def PatchSamplesBeforeBuild(args, dxil):
    for sample_name in sample_names:
        PatchSample(args, sample_name, dxil, False)


def RunSampleTests(args):
    CheckEnvironment(args)
    print("Building Debug config ...")
    BuildSamples(args, False)

    print("Building DxilDebug config ...")
    BuildSamples(args, True)

    print("Applying patch to post-build binaries to enable dxc ...")
    PatchSamplesAfterBuild(args, False)
    PatchSamplesAfterBuild(args, True)

    print(
        "TODO - run Debug config vs. DxilDebug config and verify results are the same"
    )


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Run D3D sample tests...")
    parser.add_argument("-x86", action="store_true", help="add x86 targets")
    parser.add_argument("-samples", help="path to root of D3D12 samples")
    parser.add_argument("-bins", help="path to dxcompiler.dll and related binaries")
    parser.add_argument(
        "-sample", help="choose a single sample to build/test (* wildcard supported)"
    )
    parser.add_argument(
        "-postbuild", action="store_true", help="only perform post-build operations"
    )
    parser.add_argument("-patch", action="store_true", help="patch projects")
    parser.add_argument(
        "-symbols",
        action="store_true",
        help="try to copy symbols for various dependencies",
    )
    args = parser.parse_args()

    SetSampleActions()

    if args.x86:
        args.arch = "Win32"
    else:
        args.arch = "x64"

    if not args.samples:
        print("The -samples option must be used to indicate the root of D3D12 Samples.")
        print("Samples are available at this URL.")
        print("https://github.com/Microsoft/DirectX-Graphics-Samples")
        exit(1)

    if args.sample:
        print("Applying sample filter: %s" % args.sample)
        args.sample = re.escape(args.sample).replace("\\*", ".*")
        rxSample = re.compile(args.sample, re.I)
        for name in samples:
            if rxSample.match(name):
                print("  %s" % name)

    if args.postbuild:
        print("Applying patch to post-build binaries to enable dxc ...")
        PatchSamplesAfterBuild(args, False)
        PatchSamplesAfterBuild(args, True)
    elif args.patch:
        print("Patching projects ...")
        PatchProjects(args)
    else:
        RunSampleTests(args)
