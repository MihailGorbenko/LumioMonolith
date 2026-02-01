# Trim trailing whitespace and remove extra empty lines at EOF for source files
# Usage: run in repo root: powershell -ExecutionPolicy Bypass -File .\tools\cleanup_whitespace.ps1

$exts = "*.cpp","*.c","*.hpp","*.h","*.ino"
Get-ChildItem -Recurse -Include $exts | ForEach-Object {
    $path = $_.FullName
    $text = Get-Content -Raw -Encoding UTF8 $path
    # Normalize CRLF
    $text = $text -replace "\r\n", "`n"
    # Trim trailing spaces on each line
    $lines = $text -split "`n" | ForEach-Object { $_ -replace "[ \t]+$", "" }
    # Remove extra blank lines at EOF
    while ($lines.Count -gt 0 -and ($lines[-1] -eq "")) { $lines = $lines[0..($lines.Count-2)] }
    $out = ($lines -join "`r`n") + "`r`n"
    Set-Content -Encoding UTF8 -NoNewline -Path $path -Value $out
    Write-Host "Cleaned: $path"
}
Write-Host "Whitespace cleanup complete." -ForegroundColor Green
