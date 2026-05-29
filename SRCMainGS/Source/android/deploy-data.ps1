# deploy-data.ps1 — Copy Mu Online game data to Android device/emulator
#
# Usage:
#   .\deploy-data.ps1                    # Deploy all data to /sdcard/Data/
#   .\deploy-data.ps1 -Minimal           # Only essential files (keys, music)
#   .\deploy-data.ps1 -AdbArgs "-s emulator-5554"  # Target specific device
#
# The game resolves relative paths against /sdcard/. Common paths:
#   Data/Local/Dec2.dat     → /sdcard/Data/Local/Dec2.dat
#   Data/Music/Pub.mp3      → /sdcard/Data/Music/Pub.mp3 (BGM via MediaPlayer)
#   Data/World2/TerrainHeight.OZB → /sdcard/Data/World2/TerrainHeight.OZB
#   ...
#
# Source: assets_Data_backup/ contains the full Data/ directory tree contents.

param(
    [switch]$Minimal,
    [string]$AdbArgs = ""
)

$ErrorActionPreference = "Stop"
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$SourceDir = Join-Path $ScriptDir "app\src\main\assets_Data_backup"
$DevicePath = "/sdcard/Data/"

# Verify adb is available
$adb = Get-Command adb -ErrorAction SilentlyContinue
if (-not $adb) {
    Write-Error "adb not found. Install Android SDK Platform Tools."
    exit 1
}

# Verify source directory
if (-not (Test-Path $SourceDir)) {
    Write-Error "Source directory not found: $SourceDir"
    exit 1
}

Write-Host "=== Mu Online Game Data Deploy ==="
Write-Host "Source: $SourceDir"
Write-Host "Target: $DevicePath (on device)"
Write-Host ""

# Check device connection
$devices = adb devices 2>$null | Select-String -Pattern "device$"
if (-not $devices) {
    Write-Error "No Android device/emulator connected. Run 'adb devices' to check."
    exit 1
}
Write-Host "Connected devices:"
$devices | ForEach-Object { Write-Host "  $_" }
Write-Host ""

# Build adb command prefix
$adbCmd = "adb"
if ($AdbArgs) { $adbCmd += " $AdbArgs" }

if ($Minimal) {
    # --- Minimal: only critical files ---
    Write-Host "[Minimal deploy] Copying critical files..."

    $critical = @(
        "Dec2.dat",
        "Enc1.dat",
        "Local"
    )

    foreach ($item in $critical) {
        $src = Join-Path $SourceDir $item
        if (Test-Path $src) {
            Write-Host "  Pushing $item..."
            & cmd /c "$adbCmd push `"$src`" $DevicePath 2>&1"
        } else {
            Write-Warning "  Not found: $item"
        }
    }

    # Also push Music directory for BGM
    $musicSrc = Join-Path $SourceDir "Music"
    if (Test-Path $musicSrc) {
        Write-Host "  Pushing Music/ (BGM files)..."
        & cmd /c "$adbCmd push `"$musicSrc`" $DevicePath 2>&1"
    }
} else {
    # --- Full deploy: entire Data directory ---
    Write-Host "[Full deploy] This will copy ~2.4 GB of game data."
    Write-Host "Press Ctrl+C to cancel, or Enter to continue..."
    Read-Host

    # Push the entire Data directory to /sdcard/
    # We use a two-step approach:
    # 1. Create tar on host, push tar, extract on device (faster for many files)
    # 2. Fallback: adb push directory

    Write-Host "Pushing full game data (this may take several minutes)..."

    # Push each top-level subdirectory to avoid path nesting issues
    Get-ChildItem -Path $SourceDir -Directory | ForEach-Object {
        $name = $_.Name
        $src = $_.FullName
        Write-Host "  Pushing $name/ ..."
        & cmd /c "$adbCmd push `"$src`" $DevicePath 2>&1"
    }

    # Push top-level files
    Get-ChildItem -Path $SourceDir -File | ForEach-Object {
        $name = $_.Name
        $src = $_.FullName
        Write-Host "  Pushing $name ..."
        & cmd /c "$adbCmd push `"$src`" $DevicePath 2>&1"
    }
}

Write-Host ""
Write-Host "=== Deploy complete ==="
Write-Host "Game data is at $DevicePath on the device."

# Create .nomedia to prevent Android media scanner from indexing game assets
& cmd /c "$adbCmd shell touch ${DevicePath}.nomedia 2>&1" | Out-Null

Write-Host "Created ${DevicePath}.nomedia (prevents media scanner indexing)"
Write-Host ""
Write-Host "Verification: adb shell ls $DevicePath"
& cmd /c "$adbCmd shell ls $DevicePath 2>&1"
Write-Host ""
Write-Host "Embedded APK assets (Dec2.dat, Enc1.dat, Connect.msil) are auto-extracted on first run."
Write-Host "Use -Minimal flag for quick iteration during development."
