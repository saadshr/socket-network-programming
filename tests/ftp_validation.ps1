$ErrorActionPreference = 'Stop'

$repo = Split-Path -Parent $PSScriptRoot
$serverExe = Join-Path $repo 'build\ftp_server.exe'
$clientExe = Join-Path $repo 'build\ftp_client.exe'
$root = Join-Path $repo 'ftp_root'
$build = Join-Path $repo 'build'
$results = [System.Collections.Generic.List[object]]::new()
$server = $null

function Add-Result($Name, $Passed, $Evidence) {
    $results.Add([pscustomobject]@{
        Test = $Name
        Result = if ($Passed) { 'PASS' } else { 'FAIL' }
        Evidence = $Evidence
    })
}

function Start-FtpServer {
    $info = [Diagnostics.ProcessStartInfo]::new()
    $info.FileName = $serverExe
    $info.WorkingDirectory = $repo
    $info.UseShellExecute = $false
    $info.RedirectStandardOutput = $true
    $info.RedirectStandardError = $true
    $info.Environment['SOCKETHUB_FTP_USER'] = 'student'
    $info.Environment['SOCKETHUB_FTP_PASSWORD'] = 'secret'
    $process = [Diagnostics.Process]::Start($info)
    Start-Sleep -Milliseconds 500
    return $process
}

function Stop-FtpServer($Process) {
    if ($null -ne $Process) {
        $Process.Refresh()
        if (-not $Process.HasExited) {
            Stop-Process -Id $Process.Id -ErrorAction SilentlyContinue
        }
    }
}

function Read-FtpResponse($Reader) {
    $first = $Reader.ReadLine()
    if ($null -eq $first) { throw 'Connexion de contrôle fermée.' }
    $code = [int]$first.Substring(0, 3)
    if ($first.Length -ge 4 -and $first[3] -eq '-') {
        do { $line = $Reader.ReadLine() } while (-not $line.StartsWith(('{0:D3} ' -f $code)))
    }
    return [pscustomobject]@{ Code = $code; First = $first }
}

function Open-FtpSession {
    $tcp = [Net.Sockets.TcpClient]::new('127.0.0.1', 2121)
    $tcp.ReceiveTimeout = 5000
    $tcp.SendTimeout = 5000
    $stream = $tcp.GetStream()
    $reader = [IO.StreamReader]::new($stream, [Text.Encoding]::ASCII, $false, 8192, $true)
    $hello = Read-FtpResponse $reader
    if ($hello.Code -ne 220) { throw "Réponse initiale inattendue: $($hello.Code)" }
    return [pscustomobject]@{ Tcp = $tcp; Stream = $stream; Reader = $reader }
}

function Send-FtpCommand($Session, $Command) {
    $bytes = [Text.Encoding]::ASCII.GetBytes("$Command`r`n")
    $Session.Stream.Write($bytes, 0, $bytes.Length)
    $Session.Stream.Flush()
    return Read-FtpResponse $Session.Reader
}

function Open-PasvData($Session) {
    $response = Send-FtpCommand $Session 'PASV'
    $match = [regex]::Match($response.First, '\((\d+),(\d+),(\d+),(\d+),(\d+),(\d+)\)')
    if ($response.Code -ne 227 -or -not $match.Success) { throw 'Réponse PASV invalide.' }
    $address = '{0}.{1}.{2}.{3}' -f $match.Groups[1].Value, $match.Groups[2].Value,
        $match.Groups[3].Value, $match.Groups[4].Value
    $port = [int]$match.Groups[5].Value * 256 + [int]$match.Groups[6].Value
    return [Net.Sockets.TcpClient]::new($address, $port)
}

function Read-Data($DataTcp) {
    $memory = [IO.MemoryStream]::new()
    $buffer = New-Object byte[] 65536
    $stream = $DataTcp.GetStream()
    while (($count = $stream.Read($buffer, 0, $buffer.Length)) -gt 0) {
        $memory.Write($buffer, 0, $count)
    }
    $DataTcp.Close()
    return $memory.ToArray()
}

function Write-Data($DataTcp, [byte[]]$Bytes) {
    $stream = $DataTcp.GetStream()
    $stream.Write($Bytes, 0, $Bytes.Length)
    $stream.Close()
    $DataTcp.Close()
}

function Same-Bytes([byte[]]$Left, [byte[]]$Right) {
    if ($Left.Length -ne $Right.Length) { return $false }
    for ($i = 0; $i -lt $Left.Length; $i++) {
        if ($Left[$i] -ne $Right[$i]) { return $false }
    }
    return $true
}

try {
    Remove-Item (Join-Path $root 'validation-seed.txt'), (Join-Path $root 'validation-text.txt'),
        (Join-Path $root 'validation-binary.bin'), (Join-Path $root 'client-upload.txt'),
        (Join-Path $root 'link-out'), (Join-Path $build 'client-source.txt'),
        (Join-Path $build 'client-download.txt') -Force -Recurse -ErrorAction SilentlyContinue

    [IO.File]::WriteAllText((Join-Path $root 'validation-seed.txt'), 'seed for LIST')
    [IO.File]::WriteAllText((Join-Path $build 'client-source.txt'), 'upload from C client')
    $text = [Text.Encoding]::UTF8.GetBytes("SocketHub-C FTP`r`nDeuxième ligne`r`n")
    [byte[]]$binary = 0..255
    $binary = $binary * 1024
    $junctionCreated = $false
    try {
        New-Item -ItemType Junction -Path (Join-Path $root 'link-out') -Target $build -ErrorAction Stop | Out-Null
        $junctionCreated = $true
    } catch { }

    $server = Start-FtpServer
    $session = Open-FtpSession
    $codes = @(
        (Send-FtpCommand $session 'USER student').Code,
        (Send-FtpCommand $session 'PASS secret').Code,
        (Send-FtpCommand $session 'SYST').Code,
        (Send-FtpCommand $session 'FEAT').Code,
        (Send-FtpCommand $session 'TYPE I').Code,
        (Send-FtpCommand $session 'PWD').Code,
        (Send-FtpCommand $session 'NOOP').Code
    )
    Add-Result 'authentification et commandes de contrôle' (($codes -join ',') -eq '331,230,215,211,200,257,200') "codes=$($codes -join ',')"

    $data = Open-PasvData $session
    $start = Send-FtpCommand $session 'LIST'; $listing = [Text.Encoding]::UTF8.GetString((Read-Data $data)); $finish = Read-FtpResponse $session.Reader
    Add-Result 'PASV + LIST' ($start.Code -eq 150 -and $finish.Code -eq 226 -and $listing.Contains('validation-seed.txt')) "codes=$($start.Code)/$($finish.Code)"

    $data = Open-PasvData $session; $start = Send-FtpCommand $session 'STOR validation-text.txt'; Write-Data $data $text; $finish = Read-FtpResponse $session.Reader
    Add-Result 'STOR texte' ($start.Code -eq 150 -and $finish.Code -eq 226) "codes=$($start.Code)/$($finish.Code)"

    $data = Open-PasvData $session; $start = Send-FtpCommand $session 'RETR validation-text.txt'; $downloadedText = Read-Data $data; $finish = Read-FtpResponse $session.Reader
    Add-Result 'RETR texte' ($start.Code -eq 150 -and $finish.Code -eq 226 -and (Same-Bytes $downloadedText $text)) "octets=$($downloadedText.Length)"

    $data = Open-PasvData $session; $start = Send-FtpCommand $session 'STOR validation-binary.bin'; Write-Data $data $binary; $finish = Read-FtpResponse $session.Reader
    Add-Result 'STOR binaire' ($start.Code -eq 150 -and $finish.Code -eq 226) "codes=$($start.Code)/$($finish.Code)"

    $data = Open-PasvData $session; $start = Send-FtpCommand $session 'RETR validation-binary.bin'; $downloadedBinary = Read-Data $data; $finish = Read-FtpResponse $session.Reader
    $expectedHash = (Get-FileHash (Join-Path $root 'validation-binary.bin') -Algorithm SHA256).Hash
    $actualHash = (([Security.Cryptography.SHA256]::Create().ComputeHash($downloadedBinary) | ForEach-Object ToString x2) -join '').ToUpper()
    Add-Result 'RETR binaire + SHA-256' ($start.Code -eq 150 -and $finish.Code -eq 226 -and $expectedHash -eq $actualHash) "hash-egale=$($expectedHash -eq $actualHash)"

    foreach ($case in @(@('fichier inexistant', 'RETR missing.bin', 550), @('traversée ../', 'RETR ../README.md', 550), @('chemin absolu', 'RETR C:\Windows\win.ini', 550), @('écrasement', 'STOR validation-text.txt', 550), @('commande inconnue', 'WUT', 502))) {
        $response = Send-FtpCommand $session $case[1]
        Add-Result $case[0] ($response.Code -eq $case[2]) "code=$($response.Code)"
    }
    if ($junctionCreated) {
        $response = Send-FtpCommand $session 'LIST link-out'
        Add-Result 'reparse point' ($response.Code -eq 550) "code=$($response.Code)"
    } else { Add-Result 'reparse point' 'NOT RUN' 'junction refusé par Windows' }
    $quit = Send-FtpCommand $session 'QUIT'; Add-Result 'QUIT' ($quit.Code -eq 221) "code=$($quit.Code)"; $session.Tcp.Close()

    $bad = Open-FtpSession; $u = Send-FtpCommand $bad 'USER wrong'; $p = Send-FtpCommand $bad 'PASS wrong'; $q = Send-FtpCommand $bad 'QUIT'
    Add-Result 'identifiants invalides' ($u.Code -eq 331 -and $p.Code -eq 530 -and $q.Code -eq 221) "codes=$($u.Code),$($p.Code),$($q.Code)"; $bad.Tcp.Close()

    $menu = "student`nsecret`n1`n2`n3`n4`n5`n`n7`n$build\client-source.txt`nclient-upload.txt`n6`nclient-upload.txt`n$build\client-download.txt`n8`n9`n"
    $info = [Diagnostics.ProcessStartInfo]::new(); $info.FileName = $clientExe; $info.WorkingDirectory = $repo; $info.UseShellExecute = $false; $info.RedirectStandardInput = $true; $info.RedirectStandardOutput = $true; $info.RedirectStandardError = $true
    $client = [Diagnostics.Process]::Start($info); $client.StandardInput.Write($menu); $client.StandardInput.Close(); $null = $client.StandardOutput.ReadToEnd(); $clientError = $client.StandardError.ReadToEnd(); $client.WaitForExit()
    $same = (Get-FileHash (Join-Path $build 'client-source.txt') -Algorithm SHA256).Hash -eq (Get-FileHash (Join-Path $build 'client-download.txt') -Algorithm SHA256).Hash
    Add-Result 'client C menu + STOR/RETR' ($client.ExitCode -eq 0 -and $same) "exit=$($client.ExitCode); sha=$same; erreur=$clientError"

    $curlOutput = & curl.exe --silent --show-error --ftp-pasv --user 'student:secret' 'ftp://127.0.0.1:2121/' 2>&1
    Add-Result 'curl.exe externe' ($LASTEXITCODE -eq 0 -and ($curlOutput -join "`n").Contains('validation-seed.txt')) "exit=$LASTEXITCODE"
} catch {
    Add-Result 'harnais global' $false $_.Exception.Message
} finally {
    Stop-FtpServer $server
    Remove-Item (Join-Path $root 'validation-seed.txt'), (Join-Path $root 'validation-text.txt'), (Join-Path $root 'validation-binary.bin'), (Join-Path $root 'client-upload.txt'), (Join-Path $root 'link-out'), (Join-Path $build 'client-source.txt'), (Join-Path $build 'client-download.txt') -Force -Recurse -ErrorAction SilentlyContinue
}

$results | Format-Table -AutoSize
if ($results.Result -contains 'FAIL') { exit 1 }
