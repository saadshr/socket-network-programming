$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
$build = Join-Path $repo 'build'
& (Join-Path $PSScriptRoot 'build.ps1')
if ($LASTEXITCODE -ne 0) { throw 'La compilation a échoué.' }

Write-Host '--- Régression chat TCP ---'
$serverOut = Join-Path $build 'chat-script-server.log'
$serverErr = Join-Path $build 'chat-script-server.err'
$server = Start-Process (Join-Path $build 'chat_server.exe') -WorkingDirectory $repo -RedirectStandardOutput $serverOut -RedirectStandardError $serverErr -PassThru
try {
    Start-Sleep -Milliseconds 500
    $info = [Diagnostics.ProcessStartInfo]::new()
    $info.FileName = Join-Path $build 'chat_client.exe'; $info.WorkingDirectory = $repo; $info.UseShellExecute = $false
    $info.RedirectStandardInput = $true; $info.RedirectStandardOutput = $true; $info.RedirectStandardError = $true
    $client = [Diagnostics.Process]::Start($info)
    $client.StandardInput.Write("script alpha`nscript beta`n/quit`n"); $client.StandardInput.Close()
    $output = $client.StandardOutput.ReadToEnd(); $clientError = $client.StandardError.ReadToEnd(); $client.WaitForExit()
    $ok = ($client.ExitCode -eq 0) -and ($output -match 'script alpha') -and ($output -match 'script beta') -and ($output -match 'au revoir') -and [string]::IsNullOrWhiteSpace($clientError)
    if (-not $ok) { throw "Régression chat échouée (exit=$($client.ExitCode)). Sortie: $output Erreur: $clientError" }
    Write-Host 'Chat TCP : PASS' -ForegroundColor Green
} finally {
    $server.Refresh(); if (-not $server.HasExited) { Stop-Process -Id $server.Id -ErrorAction SilentlyContinue }
}

Write-Host '--- Validation FTP ---'
& powershell.exe -NoProfile -ExecutionPolicy Bypass -File (Join-Path $repo 'tests\ftp_validation.ps1')
if ($LASTEXITCODE -ne 0) { throw 'La validation FTP a échoué.' }
Write-Host 'Validation chat + FTP réussie.' -ForegroundColor Green
