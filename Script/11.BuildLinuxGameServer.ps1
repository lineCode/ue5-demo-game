#include configuration
. $PSScriptRoot\00.Configuration.ps1

Write-Output "---Step 11: Building Game Server (Linux) Executable---"
Invoke-Expression "${UE4BatchFile} BuildCookRun -project=${GameProjectFile} -noP4 -platform=Linux -clientconfig=Development -cook -serverconfig=Development -server -Target=LyraGame -serverplatform=Linux -noclient -build -stage -pak -archive -crashreporter -archivedirectory=${OutputLinuxGameServerFolder}"
Write-Output "---Step 11: Building Game Server (Linux) Executable---DONE"