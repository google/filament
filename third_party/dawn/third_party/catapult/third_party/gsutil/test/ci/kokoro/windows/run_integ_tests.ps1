#Requires -Version 2
#Copyright 2019 Google LLC

<#
.SYNOPSIS
  Runs gsutil integration tests in multiple processes.
.DESCRIPTION
  This script runs gsutil integration tests, performing manual parallelization.
  Note that on Linux, tests can be parallelized via the '-p <num>` option.
  However, we must manually parallelize them on Windows due to the file locking
  mechanisms in oauth2client being buggy, sometimes causing a deadlock and
  making tests hang forever. Tests are parallelized by file, meaning all tests
  for the "cp" command are run in the same process, "label" tests run in
  another process, etc.
.PARAMETER GsutilRepoDir
  This is the path to the gsutil repository. If not supplied, this script will
  check whether in current working directory for gsutil tests, looking for a
  "gslib\tests" directory hierarchy.
.PARAMETER PyExe
  This is the path to the Python executable. If not supplied, this script will
  search for a "python.exe" in the system PATH.
.PARAMETER Tests
  The names of test files or individual tests to run, separated by commas. E.g.
  to run tests in test_cp.py and test_label.py, one should pass the string
  "cp,label". To run individual cp tests, one might pass
  "cp.TestCp.test_noclobber,cp.TestCp.test_streaming".
.PARAMETER TopLevelFlags
  Top level flags to pass to gsutil, such as "-o GSUtil:gs_host=...".
  Accepts a string containing all top level flags.
#>

param (
  [string]$GsutilRepoDir,
  [string]$PyExe,
  [string]$Tests,
  [string]$TopLevelFlags
)

$SCRIPT_START_DATE = (Get-Date)  # Used to calculate runtime duration upon completion.

# Validate input params:

if (!$PyExe -or !(Test-Path -Path $PyExe)) {
  # Check for python in the system path.
  $res = Get-Command -ErrorAction SilentlyContinue 'python.exe'
  if ($res) {
    $PyExe = $res.Source
  }
  else {
    Write-Output 'Could not find Python executable. Please add it to your PATH or specify its path via the -PyExe flag.'
    exit 1
  }
}

if (!$GsutilRepoDir -or !(Test-Path -Path $GsutilRepoDir)) {
  # Check the current working directory, allowing us to run this script while
  # in the gsutil repository's root directory without specifying its path.
  $cwd = (Get-Location).Path
  if (Test-Path -Path "$cwd\gslib\tests") {
    $GsutilRepoDir = $cwd
  }
  else {
    Write-Output "Could not find gsutil repository. Please run this script while in the repository's root directory"
    Write-Output 'or specify its path via the -GsutilRepoDir flag.'
    exit 1
  }
}

# Get an array of strings like "cp", "label" from filenames like "test_cp.py",
# "test_label.py" in the "<repo-root>\gslib\tests" folder.
$ALL_TESTS = Get-ChildItem "$GsutilRepoDir\gslib\tests\test_*.py" |
    Select-Object -ExpandProperty Name |
    ForEach-Object { $_.Split('.')[0].Substring('test_'.length) }
if (!$Tests) {
  [array]$Tests = $ALL_TESTS
}
else {
  # Make sure all the specified tests are found.
  [array]$Tests = $Tests.Split(',') | ForEach-Object { $_.Trim() }
  foreach ($test in $Tests) {
    $test_file = $test.Split('.')[0]
    if (!$ALL_TESTS.Contains($test_file)) {
      Write-Output "An invalid test was specified: '$test'. Found no file named 'test_$test_file.py' in gslib\tests. Exiting."
      exit 1
    }
  }
}

# Now that we've validated all the input params and have the values we need,
# we can start the tests.

# Create a directory hierarchy in a temp folder to organize state and log dirs.
$RUN_DATE = "$(Get-Date -f yyyy-MM-dd-HH-mm-ss-ffff)"
$TEST_DIR_PATH = "$env:TMP\gsutil-test-$RUN_DATE"
$STATE_DIRS_PATH = "$TEST_DIR_PATH\state_dirs"
$LOG_DIR_PATH = "$TEST_DIR_PATH\logs"
mkdir -Force $TEST_DIR_PATH > $null
mkdir -Force $STATE_DIRS_PATH > $null
mkdir -Force $LOG_DIR_PATH > $null
Write-Output "[*] Test state & logs directory: $TEST_DIR_PATH"

$FAILURE_MSG = '<!> TEST FAILURE FOR TEST GROUP'

# Each test file should have its tests run in a separate test job.
# Keep track of jobs as we start them.
$jobs = New-Object 'System.Collections.ArrayList'
foreach ($test_group in $Tests) {
  $job = Start-Job -Name $test_group -ScriptBlock {
    mkdir -Force "$using:state_dirs_path\state_dir_$using:test_group" > $null
    & $using:PyExe "$using:GsutilRepoDir\gsutil.py" `
        -o "GSUtil:state_dir=$using:state_dirs_path\state_dir_$using:test_group" `
        $using:TopLevelFlags `
        test -p 1 $using:test_group 2>&1 |
            ForEach-Object ToString |
            Tee-Object "$using:log_dir_path\$using:test_group.log"
    # Non-zero exit code means the tests probably failed. Print a unique failure
    # message that the parent process can look for to see if the test failed.
    # Since we can't tell the exit code of a child job, this is the next best
    # thing I could come up with.
    if ($LASTEXITCODE -ne 0) {
      Write-Output "$using:FAILURE_MSG '$using:test_group'"
    }
  }
  $jobs.Add($job) > $null
}
Write-Output "[*] $($jobs.Count) jobs started."
$names = ($jobs | Select-Object -ExpandProperty Name) -join ', '
Write-Output "[*] Waiting on $($jobs.Count) test jobs: $names"

# Check each job, printing its output if it's done. If any jobs are unfinished
# after checking all of them, print the names of the remaining jobs. Repeat
# until all jobs have finished.
$some_failed = $false
while ($jobs.Count -gt 0) {
  for ($i = 0; $i -lt $jobs.Count; $i++) {
    $res = Wait-Job -Timeout 3 $jobs[$i]
    # May be null-valued if the job hasn't finished yet.
    if (!$res) { continue }
    if ($res.State.Equals('Completed') -or $res.State.Equals('Failed')) {
      Write-Output '*****************************************************************'
      Write-Output "[*] '$($res.Name)' tests finished, job Status='$($res.State)'"
      $job_output = Receive-Job -Name $res.Name
      # Only consider the test successful if the job succeeded and didn't have our
      # injected failure message in it.

      # Note that this is a System.Object[] in the event that a match is found, not
      # a boolean. But checking it via 'if ($has_fail_msg)' still yields false if no
      # match was found, and true if it matched.
      $has_fail_msg = $job_output -match $FAILURE_MSG
      if ($res.State.Equals('Completed') -and !$has_fail_msg) {
        Write-Output "[*] OK - not printing test output (didn't find failure message)"
      }
      else {
        $some_failed = $true
        Write-Output '[*] Tests failed, job output:'
        Write-Output $job_output
      }
      Write-Output '*****************************************************************'
      $jobs.RemoveAt($i)
      # Move index back one since we deleted the item previously located at
      # this index; next increment will point to the correct next item.
      $i--
    }
  }
  if ($jobs.Count -eq 0) {
    Write-Output ''
    Write-Output 'Done!'
  }
  else {
    $names = ($jobs | Select-Object -ExpandProperty Name) -join ', '
    Write-Output "[*] Waiting on $($jobs.Count) test jobs: $names"
  }
}

# Print runtime duration and exit.
$SCRIPT_END_DATE = (Get-Date)
$SCRIPT_DURATION = New-TimeSpan -Start $SCRIPT_START_DATE -End $SCRIPT_END_DATE
Write-Output "Total test time: $($SCRIPT_DURATION)"

if ($some_failed) {
  Write-Output 'Final status: FAILED'
  Exit 1
}
else {
  Write-Output 'Final status: OK'
  Exit 0
}
