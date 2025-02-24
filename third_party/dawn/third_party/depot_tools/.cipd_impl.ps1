# Copyright (c) 2016 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Note: to run this on e.g. OSX for adhoc testing or debugging in case Windows
# is not around:
#
# pwsh
# PS ...> $ExecutionContext.SessionState.LanguageMode = "ConstrainedLanguage"
# PS ...> ./.cipd_impl.ps1 -CipdBinary _cipd.exe `
#         -BackendURL https://chrome-infra-packages.appspot.com `
#         -VersionFile ./cipd_client_version
# file _cipd.exe

param(
    # Path to download the CIPD binary to.
    [Parameter(Mandatory = $true)]
    [string]
    $CipdBinary,

    # CIPD platform to download the client for.
    [string]
    $Platform = "windows-amd64",

    # E.g. "https://chrome-infra-packages.appspot.com".
    [Parameter(Mandatory = $true)]
    [string]
    $BackendURL,

    # Path to the cipd_client_version file with the client version.
    [Parameter(Mandatory = $true)]
    [string]
    $VersionFile
)

# Import PowerShell<=5 Get-Filehash from Microsoft.PowerShell.Utility.
# This prevents loading of incompatible Get-FileHash from PowerShell 6+ $PSModulePath.
# See: crbug.com/1443163.
Import-Module $PSHOME\Modules\Microsoft.PowerShell.Utility -Function Get-FileHash

$DepotToolsPath = Split-Path $MyInvocation.MyCommand.Path -Parent

# Put depot_tool's git revision into the user agent string.
try {
  $DepotToolsVersion = &git -C $DepotToolsPath rev-parse HEAD 2>&1
  if ($LastExitCode -eq 0) {
    $UserAgent = "depot_tools/$DepotToolsVersion"
  } else {
    $UserAgent = "depot_tools/???"
  }
} catch [System.Management.Automation.CommandNotFoundException] {
  $UserAgent = "depot_tools/no_git/???"
}
$Env:CIPD_HTTP_USER_AGENT_PREFIX = $UserAgent

# Returns the expected SHA256 hex digest for the given platform reading it from
# *.digests file.
function Get-Expected-SHA256($platform) {
  $digestsFile = $VersionFile+".digests"
  foreach ($line in Get-Content $digestsFile) {
    if ($line -match "^([0-9a-z\-]+)\s+sha256\s+([0-9a-f]+)$") {
      if ($Matches[1] -eq $platform) {
        return $Matches[2]
      }
    }
  }
  throw "No SHA256 digests for $platform in $digestsFile"
}

# Retry a command with a delay between each.
function Retry-Command {
  [CmdletBinding()]
  param (
      [Parameter(Mandatory = $true)]
      [scriptblock]
      $Command,

      [int]
      $MaxAttempts = 3,

      [timespan]
      $Delay = (New-TimeSpan -Seconds 5)
  )

  $attempt = 0
  while ($attempt -lt $MaxAttempts) {
    try {
      Invoke-Command -ScriptBlock $Command
      return
    }
    catch {
      $attempt += 1
      $exception = $_.Exception.InnerException
      if ($attempt -lt $MaxAttempts) {
        Write-Output "FAILURE: " + $_
        Write-Output "Retrying after a short nap..."
        Start-Sleep -Seconds $Delay.TotalSeconds
      } else {
        throw $exception
      }
    }
  }
}

# Check is url in the NO_PROXY of Environment?
function Is-UrlInNoProxy {
  [CmdletBinding()]
  param (
    [Parameter(Mandatory)]
    [string]$Url
  )
  $NoProxy = $Env:NO_PROXY
  if ([string]::IsNullOrEmpty($NoProxy)) {
    return $false
  }
  $NoProxyList = $NoProxy -split ',' | ForEach-Object { $_.Trim().ToLower() }
  $UriHost = ([uri]$Url).Host.ToLower()
  foreach ($entry in $NoProxyList) {
    if ($entry.StartsWith('.')) {
      if ($UriHost.EndsWith($entry.Substring(1))) {
        return $true
      }
    } elseif ($entry -eq $UriHost) {
      return $true
    }
  }
  return $false
}

$ExpectedSHA256 = Get-Expected-SHA256 $Platform
$Version = (Get-Content $VersionFile).Trim()
$URL = "$BackendURL/client?platform=$Platform&version=$Version"

# Fetch the binary now that the lock is ours.
$TmpPath = $CipdBinary + ".tmp." + $PID
try {
  Write-Output "Downloading CIPD client for $Platform from $URL..."

  $Parameters = @{
    UserAgent = $UserAgent
    Uri = $URL
    OutFile = $TmpPath
  }

  if ($Env:HTTPS_PROXY) {
    $Proxy = $Env:HTTPS_PROXY
  } elseif ($Env:HTTP_PROXY) {
    $Proxy = $Env:HTTP_PROXY
  } elseif ($Env:ALL_PROXY) {
    $Proxy = $Env:ALL_PROXY
  } else {
    $Proxy = $null
  }
  $UrlNotInNoProxy = -not (Is-UrlInNoProxy -Url $URL)
  if ($UrlNotInNoProxy -and $Proxy) {
    Write-Output "Using Proxy $Proxy..."
    $Parameters.Proxy = $Proxy
  }

  Retry-Command {
    $ProgressPreference = "SilentlyContinue"
    Invoke-WebRequest @Parameters
  }

  $ActualSHA256 = (Get-FileHash -Path $TmpPath -Algorithm "SHA256").Hash.toLower()
  if ($ActualSHA256 -ne $ExpectedSHA256) {
    throw "Invalid SHA256 digest: $ActualSHA256 != $ExpectedSHA256"
  }

  Move-Item -LiteralPath $TmpPath -Destination $CipdBinary -Force
} catch {
  Remove-Item -Path $TmpPath -ErrorAction Ignore
  throw  # Re raise any error
}
