#include configuration
. $PSScriptRoot\00.Configuration.ps1

# Dynamic Data
$GameClientPath = $OutputWindowsGameClientFolder + "\Windows"
$EntryPoint = "LyraGame.exe"


Write-Output "---Step 7: Uploading Game Binaries for Game client---"
# Execute the command
Invoke-Expression "${BlackBoxCLIPath} build upload-binaries --entry-point ${EntryPoint} --namespace ${Namespace} --apikey ${APIKey} --game-project=${GameProjectPath} --game-engine=${UE4GameEnginePath} --game-archive=${GameClientPath}"

Write-Output "---Step 7: Uploading Game Binaries for Game client---DONE"
