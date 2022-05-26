#include configuration
. $PSScriptRoot\00.Configuration.ps1

$Platform = "win64"

Write-Output "---Step 5: Building Game Client executable---"
Invoke-Expression "${UE4BatchFile} -ScriptsForProject=${GameProjectFile} BuildCookRun -package -pak -compressed -build -compile -cook -stage -archive -nop4 -project=${GameProjectFile} -archivedirectory=${OutputWindowsGameClientFolder} -clientconfig=Development -Target=LyraGame -targetplatform=${Platform} -utf8output -unattended -logtimes -crashreporter"
Write-Output "---Step 5: Building Game Client executable---DONE"