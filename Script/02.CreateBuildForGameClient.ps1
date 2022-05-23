#include configuration
. .\0.Configuration.ps1

# Dynamic Data
$Platform = "windows"

Write-Output "---Step 2: Creaing New Build for Game Client---"

# Execute the command
Invoke-Expression "${BlackBoxCLIPath} build register --namespace ${Namespace} --apikey ${APIKey} --game-project=${GameProjectPath} --platform-name ${Platform} --platform-arch x64 --game-engine=${UE4GameEnginePath}"
# Write-Output "${BlackBoxCLIPath} build register --namespace ${Namespace} --apikey ${APIKey} --game-project=${GameProjectPath} --platform-name ${Platform} --platform-arch x64 --game-engine=${UE4GameEnginePath} --proxy 127.0.0.1:8888"

Write-Output "---Step 2: Creaing New Build for Game Client---DONE"