# Builds a playable Windows copy of the game and zips it for sending to testers.
#
#   powershell -ExecutionPolicy Bypass -File Tools\package_game.ps1 [-Version 0.1.0]
#
# Close the Unreal editor first (the packager cooks the content itself).
# Result: <the folder next to the project>\CrownsAndCommoners_Builds\Windows (the game) and CrownsAndCommoners_v<Version>_test.zip
# Players run CrownsAndCommoners.exe; "HOW TO PLAY.txt" (from Docs) goes next to it.
param([string]$Version = "0.1.0")
$ErrorActionPreference = "Stop"
$Root = (Resolve-Path "$PSScriptRoot\..").Path          # the project folder, wherever it is
$Project = "$Root\CrownsAndCommoners.uproject"
$Out = "$(Split-Path $Root -Parent)\CrownsAndCommoners_Builds"
$RunUAT = "C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\RunUAT.bat"

& $RunUAT BuildCookRun "-project=$Project" -noP4 -platform=Win64 -clientconfig=Shipping `
    -build -cook -stage -pak -archive "-archivedirectory=$Out" -prereqs -unattended -utf8output
if ($LASTEXITCODE -ne 0) { throw "Packaging failed (exit code $LASTEXITCODE)" }

Copy-Item "$Root\Docs\HOW TO PLAY.txt" "$Out\Windows\HOW TO PLAY.txt" -Force
$Zip = "$Out\CrownsAndCommoners_v${Version}_test.zip"
Compress-Archive -Path "$Out\Windows\*" -DestinationPath $Zip -CompressionLevel Optimal -Force
"Done: $Zip (" + [int]((Get-Item $Zip).Length / 1MB) + " MB)"
