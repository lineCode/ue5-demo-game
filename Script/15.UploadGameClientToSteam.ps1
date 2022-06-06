#include configuration
. $PSScriptRoot\00.Configuration.ps1

Write-Output "---Step 15: Upload Game Client (Windows) Binaries to Steam---"




$TempCurrentDir = Get-Location
Write-Output "Current working directory: ${TempCurrentDir}"


$OutputWindowsGameClientFolderAbsolute = Resolve-Path -Path $OutputWindowsGameClientFolder


Write-Output "Copying vdf file to Artifact Folder..."
Copy-Item "${PSScriptRoot}\steam_game_build.vdf" -Destination "${OutputWindowsGameClientFolderAbsolute}"
Write-Output "Copying vdf file to Artifact Folder...DONE"


Write-Output "Changing directory to artifact: ${OutputWindowsGameClientFolderAbsolute}"
Set-Location -Path $OutputWindowsGameClientFolderAbsolute



Write-Output "Updating VDF file, replacing version placeholder with values..."
$VDFLocation="${OutputWindowsGameClientFolderAbsolute}\steam_game_build.vdf"
# Replace version placeholder with value from version
$vdfContent = Get-Content -path $VDFLocation
$vdfContent = $vdfContent.replace('<<VERSION_PLACEHOLDER>>', $Version)
Set-Content -Path $VDFLocation -Value $vdfContent -Force



Write-Output "Executing SteamCMD..."
Invoke-Expression "${SteamCDMPath} +login ${SteamAccoutName} ${SteamAccoutPassword} +run_app_build ${VDFLocation} +quit"

Write-Output "Changing directory back to previous directory: ${TempCurrentDir}"
Set-Location -Path $TempCurrentDir


Write-Output "---Step 15: Upload Game Client (Windows) Binaries to Steam---DONE"