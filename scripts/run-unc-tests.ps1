# Runs openscad.exe on small test projects reached through local, UNC,
# mapped-drive and cross-drive paths, and records whether each run succeeds,
# fails or crashes. See https://github.com/openscad/openscad/issues/5425.
#
# Results are written to <OutDir>/results.json, and the full output of each
# run to <OutDir>/logs/.

param(
  [Parameter(Mandatory)] [string]$Exe,
  [Parameter(Mandatory)] [string]$Variant,
  [Parameter(Mandatory)] [string]$OutDir
)

$ErrorActionPreference = 'Continue'
$LogDir = Join-Path $OutDir 'logs'
New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
$EchoDir = 'C:\unc-out'
New-Item -ItemType Directory -Force -Path $EchoDir | Out-Null

# --- Test files ----------------------------------------------------------

$Files = [ordered]@{
  'dummy1.scad'  = 'module create_cube() { echo("create_cube called"); cube(10, center=true); }'
  'use.scad'     = "use <dummy1.scad>`ncreate_cube();`necho(`"main done`");"
  'include.scad' = "include <dummy1.scad>`ncreate_cube();`necho(`"main done`");"
  'warning.scad' = "echo(undefined_variable);`necho(`"main done`");"
  'error.scad'   = "echo(`"main done`");`nx"
}

# Case name -> file to open, and the patterns all of which must appear in the
# output for the run to count as OK.
$Cases = [ordered]@{
  'use'     = @{ File = 'use.scad';     Expect = @('ECHO: "create_cube called"', 'ECHO: "main done"') }
  'include' = @{ File = 'include.scad'; Expect = @('ECHO: "create_cube called"', 'ECHO: "main done"') }
  'warning' = @{ File = 'warning.scad'; Expect = @('Ignoring unknown variable', 'warning.scad, line 1', 'ECHO: "main done"') }
  'error'   = @{ File = 'error.scad';   Expect = @('Parser error', 'error.scad, line') }
}

function New-TestProject([string]$Dir) {
  New-Item -ItemType Directory -Force -Path $Dir | Out-Null
  foreach ($name in $Files.Keys) {
    Set-Content -Path (Join-Path $Dir $name) -Value $Files[$name] -NoNewline
  }
}

$LocalDir = 'C:\unc-test\proj'
$OtherDriveDir = 'D:\unc-test\proj'
New-TestProject $LocalDir
New-TestProject $OtherDriveDir

# --- Locations -----------------------------------------------------------

$shareOk = $false
try {
  if (-not (Get-SmbShare -Name unctest -ErrorAction SilentlyContinue)) {
    New-SmbShare -Name unctest -Path $LocalDir -FullAccess Everyone -ErrorAction Stop | Out-Null
  }
  $shareOk = $true
} catch {
  Write-Warning "Could not create SMB share: $_"
}

$mappedOk = $false
if ($shareOk) {
  & net use Z: '\\localhost\unctest' | Out-Host
  $mappedOk = ($LASTEXITCODE -eq 0) -and (Test-Path 'Z:\use.scad')
}

# Name -> directory the files are opened from, working directory for the
# process, and whether files are passed as absolute paths or relative to the
# working directory.
$Locations = [ordered]@{
  'local'       = @{ Dir = $LocalDir;                       Cwd = 'C:\';                  Relative = $false; Available = $true }
  'unc-admin'   = @{ Dir = '\\localhost\C$\unc-test\proj'; Cwd = 'C:\';                  Relative = $false; Available = $true }
  'unc-share'   = @{ Dir = '\\localhost\unctest';          Cwd = 'C:\';                  Relative = $false; Available = $shareOk }
  'unc-cwd'     = @{ Dir = '\\localhost\unctest';          Cwd = '\\localhost\unctest';  Relative = $true;  Available = $shareOk }
  'mapped'      = @{ Dir = 'Z:\';                           Cwd = 'C:\';                  Relative = $false; Available = $mappedOk }
  'cross-drive' = @{ Dir = $OtherDriveDir;                  Cwd = 'C:\';                  Relative = $false; Available = $true }
}

# --- Running openscad ----------------------------------------------------

function Invoke-OpenSCAD([string]$Cwd, [string]$File, [string]$OutFile) {
  $psi = [System.Diagnostics.ProcessStartInfo]::new($Exe)
  foreach ($a in @('-o', $OutFile, $File)) { $psi.ArgumentList.Add($a) }
  $psi.WorkingDirectory = $Cwd
  $psi.UseShellExecute = $false
  $psi.RedirectStandardOutput = $true
  $psi.RedirectStandardError = $true
  $p = [System.Diagnostics.Process]::Start($psi)
  $out = $p.StandardOutput.ReadToEndAsync()
  $err = $p.StandardError.ReadToEndAsync()
  if (-not $p.WaitForExit(120000)) {
    $p.Kill($true)
    return [pscustomobject]@{ Exit = 'timeout'; Stdout = ''; Stderr = '' }
  }
  $p.WaitForExit()
  return [pscustomobject]@{ Exit = $p.ExitCode; Stdout = $out.Result; Stderr = $err.Result }
}

$Results = @()
foreach ($locName in $Locations.Keys) {
  $loc = $Locations[$locName]
  foreach ($caseName in $Cases.Keys) {
    $case = $Cases[$caseName]
    $result = [ordered]@{ variant = $Variant; location = $locName; case = $caseName;
                          status = ''; exit = ''; detail = '' }
    if (-not $loc.Available -or -not (Test-Path (Join-Path $loc.Dir $case.File))) {
      $result.status = 'SKIPPED'
      $result.detail = 'location not available on this runner'
      $Results += [pscustomobject]$result
      continue
    }

    $file = if ($loc.Relative) { $case.File } else { Join-Path $loc.Dir $case.File }
    $echoFile = Join-Path $EchoDir "$locName-$caseName.echo"
    Remove-Item -Force -ErrorAction SilentlyContinue $echoFile
    $run = Invoke-OpenSCAD $loc.Cwd $file $echoFile
    $echo = if (Test-Path $echoFile) { Get-Content -Raw $echoFile } else { '' }
    $all = "$($run.Stderr)`n$($run.Stdout)`n$echo"

    $missing = @($case.Expect | Where-Object { -not $all.Contains($_) })
    $crash = $all -match 'exception|terminate called|filesystem error'
    $result.exit = "$($run.Exit)"
    $result.status = if ("$($run.Exit)" -eq 'timeout') { 'TIMEOUT' }
                     elseif ($crash) { 'CRASH' }
                     elseif ($missing.Count -gt 0) { 'FAIL' }
                     else { 'OK' }
    # The most informative line: the exception, or the WARNING/ERROR message
    # (which shows how the file name is reported).
    # For a crash, the what() line names the failing filesystem call and path.
    $lines = $all -split "`r?`n"
    $line = ($lines | Where-Object { $_ -match 'what\(\)' } | Select-Object -First 1)
    if (-not $line) {
      $line = ($lines | Where-Object {
        $_ -match 'exception|terminate called|filesystem error|WARNING|ERROR|Can''t'
      } | Select-Object -First 1)
    }
    $result.detail = if ($line) { $line.Trim() } elseif ($missing.Count -gt 0) { "missing: $($missing -join ', ')" } else { '' }

    Set-Content -Path (Join-Path $LogDir "$locName-$caseName.txt") -Value @(
      "variant:  $Variant", "location: $locName", "cwd:      $($loc.Cwd)", "file:     $file",
      "exit:     $($run.Exit)", "status:   $($result.status)", '',
      '--- stderr', $run.Stderr, '--- stdout', $run.Stdout, '--- echo file', $echo
    )
    $Results += [pscustomobject]$result
  }
}

if ($mappedOk) { & net use Z: /delete /y | Out-Null }

$Results | ConvertTo-Json -Depth 3 | Set-Content (Join-Path $OutDir 'results.json')
$Results | Format-Table location, case, status, exit, detail -AutoSize -Wrap | Out-String -Width 250 | Write-Host
