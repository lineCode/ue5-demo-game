#include configuration
. .\00.Configuration.ps1

# Dynamic Data
$Platform = "linux-server"


Write-Output "---Step 8: Create new build for Game server---"


# Execute the command
Invoke-Expression "${BlackBoxCLIPath} build register --namespace ${Namespace} --apikey ${APIKey} --game-project=${GameProjectPath} --platform-name ${Platform} --platform-arch x64 --game-engine=${UE4GameEnginePath}"

Write-Output "---Step 8: Create new build for Game server---DONE"
