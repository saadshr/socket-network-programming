$ErrorActionPreference = 'Stop'

$repo = Split-Path -Parent $PSScriptRoot
$build = Join-Path $repo 'build'
$gccCommand = Get-Command gcc -ErrorAction SilentlyContinue
if ($null -eq $gccCommand) {
    throw 'GCC est introuvable. Ouvrez MSYS2 UCRT64 ou ajoutez GCC au PATH.'
}

New-Item -ItemType Directory -Force -Path $build | Out-Null

function Invoke-Gcc([string[]]$Arguments) {
    Write-Host ('gcc ' + ($Arguments -join ' '))
    Push-Location $repo
    try { & $gccCommand.Source @Arguments } finally { Pop-Location }
    if ($LASTEXITCODE -ne 0) { throw "Échec de compilation GCC (code $LASTEXITCODE)." }
}

$common = @('-Wall', '-Wextra', '-Wpedantic', '-std=c11', '-Iinclude', 'src/common/net_io.c')
Invoke-Gcc ($common + @('src/server/chat_server.c', '-o', 'build/chat_server.exe', '-lws2_32'))
Invoke-Gcc ($common + @('src/client/chat_client.c', '-o', 'build/chat_client.exe', '-lws2_32'))
$ftpCommon = @('-Wall', '-Wextra', '-Wpedantic', '-std=c11', '-Isrc/ftp', '-Iinclude', 'src/common/net_io.c', 'src/ftp/ftp_common.c')
Invoke-Gcc ($ftpCommon + @('src/ftp/ftp_server.c', '-o', 'build/ftp_server.exe', '-lws2_32'))
Invoke-Gcc ($ftpCommon + @('src/ftp/ftp_client.c', '-o', 'build/ftp_client.exe', '-lws2_32'))
Write-Host 'Compilation des quatre programmes réussie.' -ForegroundColor Green
