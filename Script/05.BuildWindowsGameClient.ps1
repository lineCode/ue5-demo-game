#include configuration
. $PSScriptRoot\00.Configuration.ps1

$Platform = "win64"

Write-Output "---Step 5: Building Game Client executable---"

$GameClientPathAbsolute = Resolve-Path -Path $OutputWindowsGameClientFolder

# Write-Output "DELETING ${GameClientPathAbsolute}..."
# Get-ChildItem -Path "${GameClientPathAbsolute}" *.* -Recurse | foreach { $_.Delete()}
# Write-Output "DELETING ${GameClientPathAbsolute}...DONE"

Remove-Item "${GameClientPathAbsolute}\*" -Recurse -Force

Invoke-Expression "${UE4BatchFile} -ScriptsForProject=${GameProjectFile} BuildCookRun -package -pak -compressed -build -compile -cook -stage -archive -nop4 -project=${GameProjectFile} -archivedirectory=${OutputWindowsGameClientFolder} -clientconfig=Development -targetplatform=${Platform} -utf8output -unattended -logtimes -crashreporter"
Write-Output "---Step 5: Building Game Client executable---DONE"