#include configuration
. $PSScriptRoot\00.Configuration.ps1

# Dynamic Data
$Platform = "windows"

# Execute the command
Write-Output "---Step 2: Creaing New Build for Game Client---"
Invoke-Expression "${BlackBoxCLIPath} build register --namespace ${Namespace} --apikey ${APIKey} --game-project=${GameProjectPath} --platform-name ${Platform} --platform-arch x64 --game-engine=${UE4GameEnginePath}"
Write-Output "---Step 2: Creaing New Build for Game Client---DONE"