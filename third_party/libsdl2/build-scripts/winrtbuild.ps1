#
# winrtbuild.ps1 -- A Powershell script to build all SDL/WinRT variants,
#    across all WinRT platforms, in all of their supported, CPU architectures.
#
# Initial version written by David Ludwig <dludwig@pobox.com>
#
# This script can be launched from Windows Explorer by double-clicking
# on winrtbuild.bat
#
# Output will be placed in the following subdirectories of the SDL source
# tree:
#   * VisualC-WinRT\lib\  -- final .dll, .lib, and .pdb files
#   * VisualC-WinRT\obj\  -- intermediate build files
#
# Recommended Dependencies:
#   * Windows 8.1 or higher
#   * Powershell 4.0 or higher (included as part of Windows 8.1)
#   * Visual C++ 2012, for building Windows 8.0 and Windows Phone 8.0 binaries.
#   * Visual C++ 2013, for building Windows 8.1 and Windows Phone 8.1 binaries
#   * SDKs for Windows 8.0, Windows 8.1, Windows Phone 8.0, and
#     Windows Phone 8.1, as needed
#
# Commom parameters/variables may include, but aren't strictly limited to:
#   * PlatformToolset: the name of one of Visual Studio's build platforms.
#     Different PlatformToolsets output different binaries.  One
#     PlatformToolset exists for each WinRT platform.  Possible values
#     may include:
#       - "v110": Visual Studio 2012 build tools, plus the Windows 8.0 SDK
#       - "v110_wp80": Visual Studio 2012 build tools, plus the Windows Phone 8.0 SDK
#       - "v120": Visual Studio 2013 build tools, plus the Windows 8.1 SDK
#       - "v120_wp81": Visual Studio 2013 build tools, plus the Windows Phone 8.1 SDK
#   * VSProjectPath: the full path to a Visual Studio or Visual C++ project file
#   * VSProjectName: the internal name of a Visual Studio or Visual C++ project
#     file.  Some of Visual Studio's own build tools use this name when
#     calculating paths for build-output.
#   * Platform: a Visual Studio platform name, which often maps to a CPU
#     CPU architecture.  Possible values may include: "Win32" (for 32-bit x86),
#     "ARM", or "x64" (for 64-bit x86).
#

# Base version of SDL, used for packaging purposes
$SDLVersion = "2.0.16"

# Gets the .bat file that sets up an MSBuild environment, given one of
# Visual Studio's, "PlatformToolset"s.
function Get-MSBuild-Env-Launcher
{
    param(
        [Parameter(Mandatory=$true,Position=1)][string]$PlatformToolset
    )

    if ($PlatformToolset -eq "v110") {      # Windows 8.0 (not Windows Phone), via VS 2012
        return "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\vcvarsall.bat"
    }
    if ($PlatformToolset -eq "v110_wp80") { # Windows Phone 8.0, via VS 2012
        return "C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\WPSDK\WP80\vcvarsphoneall.bat"
    }
    if ($PlatformToolset -eq "v120") {      # Windows 8.1 (not Windows Phone), via VS 2013
        return "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat"
    }
    if ($PlatformToolset -eq "v120_wp81") { # Windows Phone 8.1, via VS 2013
        return "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat"
    }
    if ($PlatformToolset -eq "v140") {      # Windows 10, via VS 2015
        return "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat"
    }
    return ""
}

# Gets a string that identifies the build-variant of SDL/WinRT that is specific
# to a particular Visual Studio PlatformToolset.
function Get-SDL-WinRT-Variant-Name
{
    param(
        [Parameter(Mandatory=$true,Position=1)][string]$PlatformToolset,

        # If true, append a string to this function's output, identifying the
        # build-variant's minimum-supported version of Visual Studio.
        [switch]$IncludeVSSuffix = $false
    )

    if ($PlatformToolset -eq "v110") {      # Windows 8.0 (not Windows Phone), via VS 2012 project files
        if ($IncludeVSSuffix) {
            return "WinRT80_VS2012"
        } else {
            return "WinRT80"
        }
    }
    if ($PlatformToolset -eq "v110_wp80") { # Windows Phone 8.0, via VS 2012 project files
        if ($IncludeVSSuffix) {
            return "WinPhone80_VS2012"
        } else {
            return "WinPhone80"
        }
    }
    if ($PlatformToolset -eq "v120") {      # Windows 8.1 (not Windows Phone), via VS 2013 project files
        if ($IncludeVSSuffix) {
            return "WinRT81_VS2013"
        } else {
            return "WinRT81"
        }
    }
    if ($PlatformToolset -eq "v120_wp81") { # Windows Phone 8.1, via VS 2013 project files
        if ($IncludeVSSuffix) {
            return "WinPhone81_VS2013"
        } else {
            return "WinPhone81"
        }
    }
    if ($PlatformToolset -eq "v140") {      # Windows 10, via VS 2015 project files
        if ($IncludeVSSuffix) {
            return "UWP_VS2015"
        } else {
            return "UWP"
        }
    }
    return ""
}

# Returns the internal name of a Visual Studio Project.
#
# The internal name of a VS Project is encoded inside the project file
# itself, inside a set of <ProjectName></ProjectName> XML tags.
function Get-VS-ProjectName
{
    param(
        [Parameter(Mandatory=$true,Position=1)]$VSProjectPath
    )

    # For now, just do a regex for the project name:
    $matches = (Get-Content $VSProjectPath | Select-String -Pattern ".*<ProjectName>([^<]+)<.*").Matches
    foreach ($match in $matches) {
        if ($match.Groups.Count -ge 1) {
            return $match.Groups[1].Value
        }
    }
    return $null
}

# Build a specific variant of SDL/WinRT
function Build-SDL-WinRT-Variant
{
    #
    # Read in arguments:
    #
    param (
        # name of an SDL project file, minus extensions and
        # platform-identifying suffixes
        [Parameter(Mandatory=$true,Position=1)][string]$SDLProjectName,

        [Parameter(Mandatory=$true,Position=2)][string]$PlatformToolset,

        [Parameter(Mandatory=$true,Position=3)][string]$Platform
    )

    #
    # Derive other properties from read-in arguments:
    #

    # The .bat file to setup a platform-appropriate MSBuild environment:
    $BatchFileForMSBuildEnv = Get-MSBuild-Env-Launcher $PlatformToolset

    # The full path to the VS Project that'll be built:
    $VSProjectPath = "$PSScriptRoot\..\VisualC-WinRT\$(Get-SDL-WinRT-Variant-Name $PlatformToolset -IncludeVSSuffix)\$SDLProjectName-$(Get-SDL-WinRT-Variant-Name $PlatformToolset).vcxproj"

    # The internal name of the VS Project, used in some post-build steps:
    $VSProjectName = Get-VS-ProjectName $VSProjectPath

    # Where to place output binaries (.dll, .lib, and .pdb files):
    $OutDir = "$PSScriptRoot\..\VisualC-WinRT\lib\$(Get-SDL-WinRT-Variant-Name $PlatformToolset)\$Platform"

    # Where to place intermediate build files:
    $IntermediateDir = "$PSScriptRoot\..\VisualC-WinRT\obj\$SDLProjectName-$(Get-SDL-WinRT-Variant-Name $PlatformToolset)\$Platform"

    #
    # Build the VS Project:
    #
    cmd.exe /c " ""$BatchFileForMSBuildEnv"" x86 & msbuild ""$VSProjectPath"" /p:Configuration=Release /p:Platform=$Platform /p:OutDir=""$OutDir\\"" /p:IntDir=""$IntermediateDir\\""" | Out-Host
    $BuildResult = $?

    #
    # Move .dll files into place.  This fixes a problem whereby MSBuild may
    # put output files into a sub-directory of $OutDir, rather than $OutDir
    # itself.
    #
    if (Test-Path "$OutDir\$VSProjectName\") {
        Move-Item -Force "$OutDir\$VSProjectName\*" "$OutDir"
    }

    #
    # Clean up unneeded files in $OutDir:
    #
    if (Test-Path "$OutDir\$VSProjectName\") {
        Remove-Item -Recurse "$OutDir\$VSProjectName"
    }
    Remove-Item "$OutDir\*.exp"
    Remove-Item "$OutDir\*.ilk"
    Remove-Item "$OutDir\*.pri"

    #
    # All done.  Indicate success, or failure, to the caller:
    #
    #echo "RESULT: $BuildResult" | Out-Host
    return $BuildResult
}


#
# Build each variant, with corresponding .dll, .lib, and .pdb files:
#
$DidAnyDLLBuildFail = $false
$DidAnyNugetBuildFail = $false

# Ryan disabled WP8.0, because it doesn't appear to have mmdeviceapi.h that SDL_wasapi needs.
# My assumption is that no one will miss this, but send patches otherwise!  --ryan.
# Build for Windows Phone 8.0, via VC++ 2012:
#if ( ! (Build-SDL-WinRT-Variant "SDL" "v110_wp80" "ARM"))   { $DidAnyDLLBuildFail = $true }
#if ( ! (Build-SDL-WinRT-Variant "SDL" "v110_wp80" "Win32")) { $DidAnyDLLBuildFail = $true }

# Build for Windows Phone 8.1, via VC++ 2013:
if ( ! (Build-SDL-WinRT-Variant "SDL" "v120_wp81" "ARM"))   { $DidAnyDLLBuildFail = $true }
if ( ! (Build-SDL-WinRT-Variant "SDL" "v120_wp81" "Win32")) { $DidAnyDLLBuildFail = $true }

# Build for Windows 8.0 and Windows RT 8.0, via VC++ 2012:
#
# Win 8.0 auto-building was disabled on 2017-Feb-25, by David Ludwig <dludwig@pobox.com>.
# Steam's OS-usage surveys indicate that Windows 8.0 use is pretty much nil, plus
# Microsoft hasn't supported Windows 8.0 development for a few years now.
# The commented-out lines below may still work on some systems, though.
# 
#if ( ! (Build-SDL-WinRT-Variant "SDL" "v110" "ARM"))        { $DidAnyDLLBuildFail = $true }
#if ( ! (Build-SDL-WinRT-Variant "SDL" "v110" "Win32"))      { $DidAnyDLLBuildFail = $true }
#if ( ! (Build-SDL-WinRT-Variant "SDL" "v110" "x64"))        { $DidAnyDLLBuildFail = $true }

# Build for Windows 8.1 and Windows RT 8.1, via VC++ 2013:
if ( ! (Build-SDL-WinRT-Variant "SDL" "v120" "ARM"))        { $DidAnyDLLBuildFail = $true }
if ( ! (Build-SDL-WinRT-Variant "SDL" "v120" "Win32"))      { $DidAnyDLLBuildFail = $true }
if ( ! (Build-SDL-WinRT-Variant "SDL" "v120" "x64"))        { $DidAnyDLLBuildFail = $true }

# Build for Windows 10, via VC++ 2015
if ( ! (Build-SDL-WinRT-Variant "SDL" "v140" "ARM"))        { $DidAnyDLLBuildFail = $true }
if ( ! (Build-SDL-WinRT-Variant "SDL" "v140" "Win32"))      { $DidAnyDLLBuildFail = $true }
if ( ! (Build-SDL-WinRT-Variant "SDL" "v140" "x64"))        { $DidAnyDLLBuildFail = $true }

# Build NuGet packages, if possible
if ($DidAnyDLLBuildFail -eq $true) {
    Write-Warning -Message "Unable to build all variants.  NuGet packages will not be built."
    $DidAnyNugetBuildFail = $true
} else {
    $NugetPath = (Get-Command -CommandType Application nuget.exe | %{$_.Path}) 2> $null
    if ("$NugetPath" -eq "") {
        Write-Warning -Message "Unable to find nuget.exe.  NuGet packages will not be built."
        $DidAnyNugetBuildFail = $true
    } else {
        Write-Host -ForegroundColor Cyan "Building SDL2 NuGet packages..."
        Write-Host -ForegroundColor Cyan "... via NuGet install: $NugetPath"
        $NugetOutputDir = "$PSScriptRoot\..\VisualC-WinRT\lib\nuget"
        Write-Host -ForegroundColor Cyan "...  output directory: $NugetOutputDir"
        $SDLHGRevision = $($(hg log -l 1 --repository "$PSScriptRoot\.." | select-string "changeset") -Replace "changeset:\W*(\d+).*",'$1') 2>$null
        Write-Host -ForegroundColor Cyan "...       HG Revision: $SDLHGRevision"

        # Base options to nuget.exe
        $NugetOptions = @("pack", "PACKAGE_NAME_WILL_GO_HERE", "-Output", "$NugetOutputDir")

        # Try attaching hg revision to NuGet package:
        $NugetOptions += "-Version"
        if ("$SDLHGRevision" -eq "") {
            Write-Warning -Message "Unable to find the Mercurial revision (maybe hg.exe can't be found?).  NuGet packages will not have this attached to their name."
            $NugetOptions += "$SDLVersion-Unofficial"
        } else {
            $NugetOptions += "$SDLVersion.$SDLHGRevision-Unofficial"
        }

        # Create NuGet output dir, if not yet created:
        if ($(Test-Path "$NugetOutputDir") -eq $false) {
            New-Item "$NugetOutputDir" -type directory
        }

        # Package SDL2:
        $NugetOptions[1] = "$PSScriptRoot\..\VisualC-WinRT\SDL2-WinRT.nuspec"
        &"$NugetPath" $NugetOptions -Symbols
        if ( ! $? ) { $DidAnyNugetBuildFail = $true }

        # Package SDL2main:
        $NugetOptions[1] = "$PSScriptRoot\..\VisualC-WinRT\SDL2main-WinRT-NonXAML.nuspec"
        &"$NugetPath" $NugetOptions
        if ( ! $? ) { $DidAnyNugetBuildFail = $true }
    }
}


# Let the script's caller know whether or not any errors occurred.
# Exit codes compatible with Buildbot are used (1 for error, 0 for success).
if ($DidAnyDLLBuildFail -eq $true) {
    Write-Error -Message "Unable to build all known variants of SDL2 for WinRT"
    exit 1
} elseif ($DidAnyNugetBuildFail -eq $true) {
    Write-Warning -Message "Unable to build NuGet packages"
    exit 0  # Should NuGet package build failure lead to a non-failing result code instead?
} else {
    exit 0
}
