# Run clang-format over common C/C++ sources in the workspace
# Usage: Open PowerShell in repo root and run: .\tools\format.ps1

$clang = Get-Command clang-format -ErrorAction SilentlyContinue
if (-not $clang) {
    Write-Host "clang-format not found in PATH. Please install it (LLVM) or add to PATH." -ForegroundColor Yellow
    exit 1
}

$exts = "*.cpp","*.c","*.hpp","*.h","*.ino"
Get-ChildItem -Recurse -Include $exts | ForEach-Object {
    Write-Host "Formatting $_"
    & clang-format -i $_.FullName
}
Write-Host "Formatting complete." -ForegroundColor Green
