#include configuration
. .\0.Configuration.ps1

Write-Output "---Step 11: Building Game Server (Linux) Executable---"

$GameServerPathAbsolute = Resolve-Path -Path $OutputLinuxGameServerFolder
# Write-Output "DELETING ${GameServerPathAbsolute}..."
# Get-ChildItem -Path "${GameServerPathAbsolute}" *.* -Recurse | foreach { $_.Delete()}
# Write-Output "DELETING ${GameServerPathAbsolute}...DONE"

Remove-Item "${GameServerPathAbsolute}\*" -Recurse -Force

# Execute the command
Invoke-Expression "${UE4BatchFile} BuildCookRun -project=${GameProjectFile} -noP4 -platform=Linux -clientconfig=Development -cook -serverconfig=Development -server -serverplatform=Linux -noclient -build -stage -pak -archive -crashreporter -archivedirectory=${OutputLinuxGameServerFolder}"

Write-Output "---Step 11: Building Game Server (Linux) Executable---DONE"